#include "MicrofacetModels/AllModels.hlsli"
#include "Path-Tracing/Fresnel.hlsli"

void Model_Microfacet(
    inout RngInfo rngInfo,
    inout float3 throughput,
    float roughness,
    float metalness,

    float3 Ns,
    float3 Li,
    float3 albedo,
    float3 anisoDirAndStrength,

    float3 wo,        // V
    out float3 wi,    // L
    out float3 L_sample,

    inout float3 debug,
    inout bool hasDebugOutput)
{
    float3 T, B;
    BuildBasisFrisvad(Ns, T, B);

    L_sample = throughput * Li;

    // Convert world to shading space
    // forall X in R^3, X.z = dot(N, X)
    float3 N_s = float3(0, 0, 1);
    float3 V_s = ToDefinedSpace(wo, T, B, Ns);

    float NdV = SSpaceCosTheta(V_s);

    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metalness);
    float3 F_select = F_Schlick(NdV, F0);

    float specProb = Luminance(F_select); // kS
    specProb = clamp(specProb, 0.05f, 0.95f);

    if (cDebugForceSpecular)
        specProb = 1.0f;
    else if (cDebugForceDiffuse)
        specProb = 0.0f;

    float u1 = Rand01_Bounce(DIM_D_BSDF_U1, rngInfo);
    float u2 = Rand01_Bounce(DIM_D_BSDF_U2, rngInfo);
    float rSpecProb = Rand01_Bounce(DIM_D_SPECULAR_PROB, rngInfo);

    bool isSpecular = rSpecProb < specProb;

    if (isSpecular)
    {
        MicrofacetModel mm;
        InitializeMM(mm, roughness, rngInfo, V_s);
        if (cAnisotropyEnabled)
            InitializeMMAniso(mm, anisoDirAndStrength);

        float3 H_s = normalize(mm.Sample(u1, u2));
        float3 L_s = NormalizeSafe(reflect(-V_s, H_s), N_s);

        // Terminate ray if wi ends up inside surface
        if (L_s.z <= 0.0f)
        {
            throughput *= 0.0f;
            return;
        }

        wi = InvToDefinedSpace(L_s, T, B, Ns);

        float NdL = SSpaceCosTheta(L_s);
        float NdH = SSpaceCosTheta(H_s);
        float VdH = dot(H_s, V_s);

        float3 F = F_Schlick(VdH, F0);

        float D = mm.D(H_s);
        float G = mm.G2(L_s, V_s);
        float pdf = mm.PDF(D, H_s, V_s);

        // Torrence-Sparrow BRDF
        float3 specularBrdf = (D * G * F) / max(0.001f, 4 * NdV * NdL);
        throughput *= specularBrdf * NdL / max(0.001f, pdf) / max(0.001f, specProb);

#ifdef DEBUG_PT_INFO_OUTPUT
#     include "Debug/DebugInfoOutputMicrofacetSpec.hlsli"
#endif
        return;
    }

    // Lambertian:

    float pdf;
    float3 diffuseBrdf;
    float3 L_s;
    float3 E;
    if (cImportanceSamplingEnabled)
    {
        L_s = RandHemisphereCosineSSpace(u1, u2);
        wi = InvToDefinedSpace(L_s, T, B, Ns);

        float NdL = SSpaceCosTheta(L_s);
        pdf = NdL / PI;
        diffuseBrdf = albedo * (1.0 - metalness) / PI; // Left in for debug view
        // E = diffuseBrdf * NdL / pdf
        E = albedo * (1.0 - metalness); // Terms cancel out
    }
    else
    {
        L_s = RandHemisphereUniformSSpace(u1, u2);
        wi = InvToDefinedSpace(L_s, T, B, Ns);

        float NdL = SSpaceCosTheta(L_s);
        pdf = 1.0f / (2.0f * PI);
        diffuseBrdf = albedo * (1.0 - metalness) / PI;
        E = diffuseBrdf * NdL / max(0.001f, pdf);
    }

    E /= max(0.001f, 1.0 - specProb);
    throughput *= E;

#ifdef DEBUG_PT_INFO_OUTPUT
#     include "Debug/DebugInfoOutputMicrofacetDiff.hlsli"
#endif
}