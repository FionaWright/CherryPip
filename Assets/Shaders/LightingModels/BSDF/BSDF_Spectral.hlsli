#ifndef H_BSDF_SPECTRAL_H
#define H_BSDF_SPECTRAL_H

#include "MicrofacetModels/AllModels.hlsli"
#include "Path-Tracing/Fresnel.hlsli"

#include "LightingModels/BSDF/BRDF_Diffuse_Spectral.hlsli"
#include "LightingModels/BSDF/BRDF_Specular_Spectral.hlsli"
#include "LightingModels/BSDF/BTDF_Spectral.hlsli"

float IsReflect(inout RngInfo rngInfo, float nCurrent, float nNext, float NdV, out float reflectProb)
{
    reflectProb = Dielectric_Unpolarized(nCurrent, nNext, abs(NdV));

    if (cDebugForceReflect || CheckTIR(nCurrent, nNext, abs(NdV)))
        reflectProb = 1.0f;
    else if (cDebugForceRefract)
        reflectProb = 0.0f;

    float rReflectProb = PcgRand01(rngInfo.IndependentRngState);
    return reflectProb > rReflectProb;
}

float IsSpecular(inout RngInfo rngInfo, float nCurrent, float nNext, float NdV, out float specProb)
{
    specProb = Dielectric_Unpolarized(nCurrent, nNext, abs(NdV));
    specProb = clamp(specProb, 0.05f, 0.95f);

    if (cDebugForceSpecular)
        specProb = 1.0f;
    else if (cDebugForceDiffuse)
        specProb = 0.0f;

    float rSpecProb = Rand01_Bounce(DIM_D_SPECULAR_PROB, rngInfo);
    return rSpecProb < specProb;
}

void Model_BSDF_Spectral(
    inout RngInfo rngInfo,
    inout SpectralValue throughput,
    SpectralContext ctx,
    float roughness,
    float metalness,

    bool entering,
    bool isGlass,
    float3 sigmaA,
    float hitDist,

    float3 Ns,
    SpectralValue Li,
    SpectralValue albedo,
    float3 anisoDirAndStrength,

    float3 wo,        // V
    out float3 wi,    // L
    out SpectralValue L_sample,

    inout float3 debug,
    inout bool hasDebugOutput)
{
    float3 T, B;
    BuildBasisFrisvad(Ns, T, B);

    float3 N_s = float3(0, 0, 1);
    float3 V_s = ToDefinedSpace(wo, T, B, Ns);

    float NdV = SSpaceCosTheta(V_s);

    float nCurrent = entering ? IOR_AIR : DebugSampleIorN_Hero(ctx);
    float nNext = entering ? DebugSampleIorN_Hero(ctx) : IOR_AIR;

    float3 L_s = 0.0f;

    if (isGlass)
    {
        L_sample = CreateBlackSpectralValue(); // TODO ?

        float reflectProb;
        float isReflect = IsReflect(rngInfo, nCurrent, nNext, NdV, reflectProb);

        if (isReflect)
        {
            BRDF_Specular_Spectral(rngInfo, throughput, L_s, roughness, metalness, albedo, V_s, N_s, anisoDirAndStrength, T, B, Ns, nCurrent, nNext, debug, hasDebugOutput);
            throughput.Div(max(0.001f, reflectProb));
        }
        else
        {
            BTDF_Spectral(rngInfo, throughput, ctx, L_s, roughness, albedo, V_s, L_s, entering, nCurrent, nNext, sigmaA, hitDist, debug, hasDebugOutput);
            throughput.Div(max(0.001f, 1.0f - reflectProb));
        }

        wi = InvToDefinedSpace(L_s, T, B, Ns);
        return;
    }

    L_sample = Mul(throughput, Li);

    float specProb;
    float isSpecular = IsSpecular(rngInfo, nCurrent, nNext, NdV, specProb);

    if (isSpecular)
    {
        BRDF_Specular_Spectral(rngInfo, throughput, L_s, roughness, metalness, albedo, V_s, N_s, anisoDirAndStrength, T, B, Ns, nCurrent, nNext, debug, hasDebugOutput);
        throughput.Div(max(0.001f, specProb));
    }
    else
    {
        BRDF_Diffuse_Spectral(rngInfo, throughput, L_s, metalness, albedo, debug, hasDebugOutput);
        throughput.Div(max(0.001f, 1.0 - specProb));
    }

    wi = InvToDefinedSpace(L_s, T, B, Ns);

//#ifdef DEBUG_PT_INFO_OUTPUT
//#     include "Debug/DebugInfoOutputBSDF.hlsli"
//#endif
}

#endif