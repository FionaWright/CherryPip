#include "MicrofacetModels/AllModels.hlsli"
#include "Path-Tracing/Fresnel.hlsli"

void Model_Microfacet_Spectral(
    inout RngInfo rngInfo,
    inout SpectralValue throughput,
    float roughness,
    float metalness,
    bool entering,
    bool isGlass,
    float3 sigmaA,
    float hitDist,
    float3 Ns,
    SpectralValue Li,
    SpectralValue albedo,
    SpectralContext ctx,
    float3 anisoDirAndStrength,
    float3 wo,        // V
    out float3 wi,    // L
    out SpectralValue L_sample,
    inout float3 debug,
    inout bool hasDebugOutput)
{
    L_sample = Mul(throughput, Li);

    float3 T, B;
    BuildBasisFrisvad(Ns, T, B);

    if (!entering && isGlass)
    {
        SpectralValue sigmaASpectral;
        sigmaASpectral.FromRGB(sigmaA, eReflectance, ctx);
        sigmaASpectral.Mul(-hitDist);
        throughput.Mul(Exp(sigmaASpectral));
    }

    float3 N_s = float3(0, 0, 1);
    float3 V_s = ToDefinedSpace(wo, T, B, Ns);
    float NdV = SSpaceCosTheta(V_s);

    float nCurrent = entering ? IOR_AIR : DebugSampleIorN_Hero(ctx);
    float nNext = entering ? DebugSampleIorN_Hero(ctx) : IOR_AIR;

    if (CheckTIR(nCurrent, nNext, abs(NdV)))
    {
        float3 L_s = normalize(reflect(-V_s, N_s));
        wi = InvToDefinedSpace(L_s, T, B, Ns);
        L_sample = CreateBlackSpectralValue();
        return;
    }

    float F_Select = Dielectric_Unpolarized(nCurrent, nNext, NdV);

    float reflectProb = F_Select;
    reflectProb = clamp(reflectProb, 0.05f, 0.95f);
    if (cDebugForceReflect || !isGlass)
        reflectProb = 1.0f;
    else if (cDebugForceRefract)
        reflectProb = 0.0f;

    bool isReflect = reflectProb > PcgRand01(rngInfo.IndependentRngState);

    if (!isReflect)
    {
        float3 L_s = Refract(-V_s, N_s, nCurrent, nNext);
        wi = InvToDefinedSpace(L_s, T, B, Ns);
        L_sample = CreateBlackSpectralValue();

        throughput.Div(max(1e-6f, 1.0f - reflectProb));
#if defined(SPECTRAL_HERO_SAMPLING)
        HeroSpectrum refractWeights = ComputeHeroRefractWeights(NdV, ctx, entering);
        throughput.Value.Mul(refractWeights);
#endif
        return;
    }
    else
        throughput.Div(max(1e-6f, reflectProb));

    if (isGlass)
    {
        wi = reflect(-wo, Ns);
        L_sample = CreateBlackSpectralValue();
        return;
    }

    float specProb = F_Select;
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
            InitializeMMAniso(mm, T, B, Ns, anisoDirAndStrength);

        float3 H_s = normalize(mm.Sample(u1, u2));
        float3 L_s = NormalizeSafe(reflect(-V_s, H_s), N_s);

        wi = InvToDefinedSpace(L_s, T, B, Ns);

        float NdL = SSpaceCosTheta(L_s);
        float NdH = SSpaceCosTheta(H_s);
        float VdH = dot(H_s, V_s);

        float F = Dielectric_Unpolarized(nCurrent, nNext, VdH);

        float D = mm.D(H_s);
        float G = mm.G2(L_s, V_s);
        float pdf = mm.PDF(D, H_s, V_s);

        // Torrence-Sparrow BRDF
        float specularBrdf = (D * F * G) / max(0.001f, 4 * NdV * NdL);
        float k = NdL / max(0.001f, pdf) / max(0.001f, specProb);
        float throughputMul = specularBrdf * k;

        if (L_s.z <= 0.0f) // Terminate ray if wi ends up inside surface
            throughputMul = 0.0f;

        throughput.Mul(throughputMul);

#if defined(SPECTRAL_HERO_SAMPLING)
        HeroSpectrum reflectWeights = ComputeHeroReflectWeights(VdH, ctx, entering);
        throughput.Value.Mul(reflectWeights);
#endif

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