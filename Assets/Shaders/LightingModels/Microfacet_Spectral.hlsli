#include "MicrofacetModels/AllModels.hlsli"
#include "Spectral-Tracing/Fresnel.hlsli"

void Model_Microfacet_Spectral(
    inout RngInfo rngInfo,
    inout SpectralValue throughput,
    float roughness,
    float metalness,
    bool entering,
    float3 sigmaA,
    float hitDist,
    float3 Ns,
    SpectralValue Li,
    SpectralValue albedo,
    float lambda,
    float3 anisoDirAndStrength,
    float3 wo,        // V
    out float3 wi,    // L
    out SpectralValue L_sample,
    inout float3 debug,
    inout bool hasDebugOutput)
{
    float3 T, B;
    BuildBasisFrisvad(Ns, T, B);

    L_sample = Mul(throughput, Li);

    if (!entering && false)
    {
        SpectralValue sigmaASpectral;
        sigmaASpectral.FromRGB(sigmaA, eReflectance, lambda);
        sigmaASpectral.Mul(-hitDist);
        throughput.Mul(Exp(sigmaASpectral));
    }

    // Convert world to shading space
    // forall X in R^3, X.z = dot(N, X)
    float3 N_s = float3(0, 0, 1);
    float3 V_s = ToDefinedSpace(wo, T, B, Ns);

    float NdV = SSpaceCosTheta(V_s);

    //SpectralValue F0 = LerpFromStart(0.04, albedo, metalness);
    //SpectralValue F_select = F_Schlick_Spectral(NdV, F0);

    // TODO: Full spectrum
    float eta = SampleEta(lambda, entering);
    float R = Dielectric_Unpolarized(eta * eta, NdV);

    bool isReflect = R > PcgRand01(rngInfo.IndependentRngState);
    //isReflect = true;
    float reflectWeight = isReflect ? (1.0f / max(1e-6f, R)) : (1.0f / max(1e-6f, 1.0f - R));
    throughput.Mul(reflectWeight);

    if (!isReflect)
    {
        float3 L_s = Refract(V_s, N_s, eta);
        wi = InvToDefinedSpace(L_s, T, B, Ns);
        L_sample = CreateBlackSpectralValue();
        return;
    }

    float specProb = R * (1.0f - roughness);

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
            InitializeMMAniso(mm, T, B, Ns, anisoDirAndStrength);

        float3 H_s = normalize(mm.Sample(u1, u2));
        float3 L_s = NormalizeSafe(reflect(-V_s, H_s), N_s);

        // Terminate ray if wi ends up inside surface
        if (L_s.z <= 0.0f)
        {
            throughput.Mul(0.0f);
            return;
        }

        wi = InvToDefinedSpace(L_s, T, B, Ns);

        float NdL = SSpaceCosTheta(L_s);
        float NdH = SSpaceCosTheta(H_s);
        float VdH = dot(H_s, V_s);

        // TODO: Full spectrum
        SpectralValue F = CreateSpectralValue(Dielectric_Unpolarized(eta * eta, VdH));

        float D = mm.D(H_s);
        float G = mm.G2(L_s, V_s);
        float pdf = mm.PDF(D, H_s, V_s);

        // Torrence-Sparrow BRDF
        float fSpecularBrdf = (D * G) / max(0.001f, 4 * NdV * NdL);
        SpectralValue specularBrdf = Mul(F, fSpecularBrdf);
        float k = NdL / max(0.001f, pdf) / max(0.001f, specProb);
        throughput.Mul(Mul(specularBrdf, k));

#ifdef DEBUG_PT_INFO_OUTPUT
#     include "Spectral-Tracing/Debug/DebugInfoOutputMicrofacetSpec.hlsli"
#endif
        return;
    }

    // Lambertian:

    float pdf;
    SpectralValue diffuseBrdf;
    float3 L_s;
    SpectralValue E;
    if (cImportanceSamplingEnabled)
    {
        L_s = RandHemisphereCosineSSpace(u1, u2);
        wi = InvToDefinedSpace(L_s, T, B, Ns);

        float NdL = SSpaceCosTheta(L_s);
        pdf = NdL / PI;
        diffuseBrdf = Mul(albedo, (1.0 - metalness) / PI); // Left in for debug view
        // E = diffuseBrdf * NdL / pdf
        E = Mul(albedo, (1.0 - metalness)); // Terms cancel out
    }
    else
    {
        L_s = RandHemisphereUniformSSpace(u1, u2);
        wi = InvToDefinedSpace(L_s, T, B, Ns);

        float NdL = SSpaceCosTheta(L_s);
        pdf = 1.0f / (2.0f * PI);
        diffuseBrdf = Mul(albedo, (1.0 - metalness) / PI);
        E = Mul(diffuseBrdf, NdL / max(0.001f, pdf));
    }

    E.Div(max(0.001f, 1.0 - specProb));
    throughput.Mul(E);

#ifdef DEBUG_PT_INFO_OUTPUT
#     include "Spectral-Tracing/Debug/DebugInfoOutputMicrofacetDiff.hlsli"
#endif
}