#ifndef H_BSDF_H
#define H_BSDF_H

#include "MicrofacetModels/AllModels.hlsli"
#include "Path-Tracing/Fresnel.hlsli"

#include "LightingModels/BSDF/BRDF_Diffuse.hlsli"
#include "LightingModels/BSDF/BRDF_Specular.hlsli"
#include "LightingModels/BSDF/BTDF.hlsli"

float IsReflect(inout RngInfo rngInfo, Complex iorCurrent, Complex iorNext, float NdV, bool isConductor, out float reflectProb)
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

float IsSpecular(inout RngInfo rngInfo, float NdV, float3 F0, out float specProb)
{
    float3 F_select = F_Schlick(NdV, F0);

    specProb = Luminance(F_select); // kS
    specProb = clamp(specProb, 0.05f, 0.95f);

    if (cDebugForceSpecular)
        specProb = 1.0f;
    else if (cDebugForceDiffuse)
        specProb = 0.0f;

    float rSpecProb = Rand01_Bounce(DIM_D_SPECULAR_PROB, rngInfo);
    return rSpecProb < specProb;
}

void Model_BSDF(
    inout RngInfo rngInfo,
    inout float3 throughput,
    float roughness,
    float metalness,

    bool entering,
    bool isGlass,
    float3 sigmaA,
    float hitDist,
    float ior,

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

    float3 N_s = float3(0, 0, 1);
    float3 V_s = ToDefinedSpace(wo, T, B, Ns);

    float NdV = SSpaceCosTheta(V_s);

    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metalness);

    float3 L_s = 0.0f;

    if (isGlass)
    {
        L_sample = 0.0f; // TODO ?

        Complex iorMat = CreateComplex(ior, 0.0f);
        Complex iorCurrent = Ternary(entering, IOR_AIR, iorMat);
        Complex iorNext = Ternary(entering, iorMat, IOR_AIR);

        bool isConductor = false; // Metal glass not supported

        float reflectProb;
        float isReflect = IsReflect(rngInfo, iorCurrent, iorNext, NdV, isConductor, reflectProb);

        if (isReflect)
        {
            BRDF_Specular(rngInfo, throughput, L_s, roughness, metalness, albedo, V_s, N_s, anisoDirAndStrength, T, B, Ns, F0, debug, hasDebugOutput);
            throughput /= max(0.001f, reflectProb);
        }
        else
        {
            BTDF(rngInfo, throughput, L_s, roughness, albedo, V_s, L_s, entering, iorCurrent.Re, iorNext.Re, sigmaA, hitDist, debug, hasDebugOutput);
            throughput /= max(0.001f, 1.0f - reflectProb);
        }

        wi = InvToDefinedSpace(L_s, T, B, Ns);

#ifdef DEBUG_PT_INFO_OUTPUT
#     include "Debug/DebugInfoOutputBSDFT.hlsli"
#endif
        return;
    }

    L_sample = throughput * Li;

    float specProb;
    float isSpecular = IsSpecular(rngInfo, NdV, F0, specProb);

    if (isSpecular)
    {
        BRDF_Specular(rngInfo, throughput, L_s, roughness, metalness, albedo, V_s, N_s, anisoDirAndStrength, T, B, Ns, F0, debug, hasDebugOutput);
        throughput /= max(0.001f, specProb);
    }
    else
    {
        BRDF_Diffuse(rngInfo, throughput, L_s, metalness, albedo, debug, hasDebugOutput);
        throughput /= max(0.001f, 1.0 - specProb);
    }

    wi = InvToDefinedSpace(L_s, T, B, Ns);

#ifdef DEBUG_PT_INFO_OUTPUT
#     include "Debug/DebugInfoOutputBSDF.hlsli"
#endif
}

#endif