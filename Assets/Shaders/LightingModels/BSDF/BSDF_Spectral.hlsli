#ifndef H_BSDF_SPECTRAL_H
#define H_BSDF_SPECTRAL_H

#include "MicrofacetModels/AllModels.hlsli"
#include "Path-Tracing/Fresnel.hlsli"
#include "Spectral-Tracing/SampleIOR.hlsli"

#include "LightingModels/BSDF/BRDF_Diffuse_Spectral.hlsli"
#include "LightingModels/BSDF/BRDF_Specular_Spectral.hlsli"
#include "LightingModels/BSDF/BTDF_Spectral.hlsli"

bool IsReflect(inout RngInfo rngInfo, Complex iorCurrent, Complex iorNext, float NdV, bool isConductor, out float reflectProb)
{
    reflectProb = Fresnel_Maxwell(iorCurrent, iorNext, abs(NdV), isConductor);

    if (cDebugForceReflect)
        reflectProb = 1.0f;
    else if (!isConductor && CheckTIR(iorCurrent.Re, iorNext.Re, abs(NdV)))
        reflectProb = 1.0f;
    else if (cDebugForceRefract)
        reflectProb = 0.0f;

    float rReflectProb = PcgRand01(rngInfo.IndependentRngState);
    return reflectProb > rReflectProb;
}

bool IsSpecular(inout RngInfo rngInfo, Complex iorCurrent, Complex iorNext, float NdV, bool isConductor, out float specProb)
{
    specProb = Fresnel_Maxwell(iorCurrent, iorNext, abs(NdV), isConductor);
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
    ShadingFrame sframe = CreateShadingFrame(Ns);

    float3 N_s = float3(0, 0, 1);
    float3 V_s = sframe.ToLocal(wo);

    float NdV = SSpaceCosTheta(V_s);

	bool isConductor = IsConductor(metalness);

	Complex iorMat = SampleIor(ctx, isGlass, isConductor);
    Complex iorCurrent = Ternary(entering, IOR_AIR, iorMat);
    Complex iorNext = Ternary(entering, iorMat, IOR_AIR);

    float3 L_s = 0.0f;

    float reflectProb;
    bool isReflect = IsReflect(rngInfo, iorCurrent, iorNext, NdV, isConductor, reflectProb);

    if (isConductor && !isReflect)
    {
        throughput.Mul(0.0f);
        L_sample = CreateBlackSpectralValue();
        return;
    }

    if (isGlass)
    {
        L_sample = CreateBlackSpectralValue(); // TODO ?

        if (isReflect)
        {
            BRDF_Specular_Spectral(rngInfo, throughput, ctx, L_s, roughness, metalness, albedo, V_s, N_s, anisoDirAndStrength, sframe, iorCurrent, iorNext, entering, debug, hasDebugOutput);
            throughput.Div(max(0.001f, reflectProb));

#if defined(SPECTRAL_HERO_SAMPLING)
	        float3 H_s = normalize(L_s + V_s);
	        float VdH = dot(V_s, H_s);
            HeroSpectrum reflectWeights = ComputeHeroReflectWeights(VdH, ctx, entering, isGlass, isConductor);
            throughput.Value.Mul(reflectWeights);
#endif
        }
        else
        {
            BTDF_Spectral(rngInfo, throughput, ctx, L_s, roughness, albedo, V_s, L_s, entering, iorCurrent.Re, iorNext.Re, sigmaA, hitDist, debug, hasDebugOutput);
            throughput.Div(max(0.001f, 1.0f - reflectProb));

#if defined(SPECTRAL_HERO_SAMPLING)
            HeroSpectrum refractWeights = ComputeHeroRefractWeights(NdV, ctx, entering, isGlass, isConductor);
            throughput.Value.Mul(refractWeights);
#endif
        }

        wi = sframe.ToWorld(L_s);
        return;
    }

    L_sample = Mul(throughput, Li);

    float specProb;
    bool isSpecular = IsSpecular(rngInfo, iorCurrent, iorNext, NdV, isConductor, specProb);

    if (isSpecular)
    {
        BRDF_Specular_Spectral(rngInfo, throughput, ctx, L_s, roughness, metalness, albedo, V_s, N_s, anisoDirAndStrength, sframe, iorCurrent, iorNext, entering, debug, hasDebugOutput);
        throughput.Div(max(0.001f, specProb));
    }
    else
    {
        BRDF_Diffuse_Spectral(rngInfo, throughput, L_s, metalness, albedo, debug, hasDebugOutput);
        throughput.Div(max(0.001f, 1.0 - specProb));
    }

    wi = sframe.ToWorld(L_s);

//#ifdef DEBUG_PT_INFO_OUTPUT
//#     include "Debug/DebugInfoOutputBSDF.hlsli"
//#endif
}

#endif