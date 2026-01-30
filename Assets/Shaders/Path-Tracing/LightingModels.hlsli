#ifndef H_LIGHTING_MODELS_H
#define H_LIGHTING_MODELS_H

#include "Microfacet.hlsli"
#include "DualIncludes/HlslMath.h"
#include "MathUtils.hlsli"

void Model_LambertionDiffuse(
    inout uint rngState,
    inout float3 throughput,
    float3 Ns,
    float3 Li,
    float3 albedo,
    out float3 wi,
    out float3 L_sample)
{
    L_sample = throughput * Li;

    float3 T, B;
    BuildBasisFrisvad(Ns, T, B);

#ifdef IMPORTANCE_SAMPLING
    wi = RandHemisphereCosineWorld(rngState, T, B, Ns);
    // pdf = NdL / PI
    throughput *= albedo; // PI and pdf cancel out
#else
    wi = RandHemisphereUniformWorld(rngState, T, B, Ns);
    float pdf = 1.0f / (2.0f * PI);
    float3 diffuseBrdf = albedo / PI;
    float NdL = dot(Ns, wi);
    throughput *= diffuseBrdf * NdL / pdf;
#endif
}

void Model_Glossy(
    inout uint rngState,
    inout float3 throughput,
    float diffuseProbability,
    float roughness,
    float3 Ns,
    float3 Li,
    float3 albedo,
    float3 wo,
    out float3 wi,
    out float3 L_sample)
{
    bool isDiffuse = diffuseProbability >= PcgRand01(rngState);

    L_sample = throughput * Li;
    throughput *= lerp(float3(1, 1, 1), albedo, isDiffuse);

    float3 diffuseDir = normalize(Ns + RandDirectionSphere(rngState));
    float3 specularDir = Reflect(-wo, Ns);
    wi = lerp(specularDir, diffuseDir, roughness * isDiffuse);
}

void Model_Glass(
    inout uint rngState,
    inout float3 throughput,
    float diffuseProbability,
    float roughness,
    bool entering,
    float hitDist,
    float ior,
    float3 Ns,
    float3 Li,
    float3 albedo,
    float3 wo,
    out float3 wi,
    out float3 L_sample)
{
    if (!entering)
    {
        float attenuationDistance = 1.0f; // TODO: Unhardcode and fix issues
        float3 sigma_a = -log(albedo) / attenuationDistance;
        sigma_a = 0;
        throughput *= exp(-sigma_a * hitDist);
    }

    float iorCurrent = entering ? IOR_AIR : ior;
    float iorNext = entering ? ior : IOR_AIR;
    GlassResponse res = CalcReflectRefract(-wo, Ns, iorCurrent, iorNext);

    float3 diffuseDir = normalize(Ns + RandDirectionSphere(rngState));
    res.reflectDir = normalize(lerp(res.reflectDir, diffuseDir, diffuseProbability)); // Why diffuseprobability and not roughness?
    res.refractDir = normalize(lerp(res.refractDir, -diffuseDir, roughness));

    bool reflect = PcgRand01(rngState) <= res.reflectWeight;
    wi = reflect ? res.reflectDir : res.refractDir;

    L_sample = 0;
    throughput *= reflect ? res.reflectWeight : (1.0 - res.reflectWeight);
}

// TODO:
// Read more PBRT, go over microfacet, fresnel and importance sampling
// Try remove importance sampling from GGX to understand it better
// Find resources on the sample/pdf parts
// Get beckmann working
// Look into the anisotropic beckmann

void Model_Microfacet(
    inout uint rngState,
    inout float3 throughput,
    float roughness,
    float metalness,
    float3 Ns,
    float3 Li,
    float3 albedo,
    float3 wo,        // V
    out float3 wi,    // L
    out float3 L_sample
#ifdef DEBUG_BUFFER
    , inout float3 debug
    , inout bool hasDebugOutput
#endif
)
{
    float3 T, B;
    BuildBasisFrisvad(Ns, T, B);

    L_sample = throughput * Li;

    // Convert world to shading space
    // forall vector X, X.z = dot(N, X)
    float3 N_s = float3(0, 0, 1);
    float3 V_s = WorldToShadingSpace(wo, T, B, Ns);

    float NdV = SSpaceCosTheta(V_s);

    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metalness);
    float3 F_select = F_Schlick(NdV, F0);

    float specProb = clamp(Luminance(F_select), 0.05f, 0.95f); // kS

#if defined(FORCE_SPECULAR)
    specProb = 1.0f;
#elif defined(FORCE_DIFFUSE)
    specProb = 0.0f;
#endif

    bool isSpecular = PcgRand01(rngState) < specProb;

    if (isSpecular)
    {
#if defined(NDF_TYPE_GGX)
        float alpha = RoughnessToAlpha_GGX(roughness);
        float a2 = max(1e-6f, alpha * alpha);
        float3 H_s = SampleH_GGX(a2, rngState);
#elif defined(NDF_TYPE_BECKMANN)
        float alpha = RoughnessToAlpha_Beckmann(roughness);
        float a2 = max(1e-6f, alpha * alpha);
        float3 H_s = SampleH_Beckmann(alpha, rngState);
#else
        float alpha = 0.0f;
        float a2 = 0.0f;
        float3 H_s = 0.0f;
#endif

        float3 L_s = normalize(reflect(-V_s, H_s));
        wi = ShadingToWorldSpace(L_s, T, B, Ns);

        float NdL = SSpaceCosTheta(L_s);
        float NdH = SSpaceCosTheta(H_s);
        float VdH = dot(H_s, V_s);

        float3 F = F_Schlick(VdH, F0);

    bool pdfSampleVisibleArea = false;
#ifdef PDF_SAMPLE_VISIBLE_AREA
    pdfSampleVisibleArea = true;
#endif

#if defined(NDF_TYPE_GGX)
        float D = D_GGX(NdH, a2);
        //float Dv = D * G1_GGX(NdV, a2) * max(0.0f, HdV) / NdV;
        float G = pdfSampleVisibleArea ? G1_GGX(NdV, a2) : G_SmithGGX(NdL, NdV, a2);
        //float pdf = Pdf_GGX(D, NdH, VdH);
        // This is pdf_h: What do I do with it?
        float pdf = Pdf_General(D, G1_GGX(NdV, a2), V_s, H_s, pdfSampleVisibleArea);
#elif defined(NDF_TYPE_BECKMANN)
        float D = D_Beckmann(H_s, a2);
        float G = G_Beckmann(V_s, L_s, alpha);
        float pdf = Pdf_General(D, G1_Beckmann(V_s, alpha), V_s, H_s, pdfSampleVisibleArea);
#else
        float D = 0.0f;
        float G = 0.0f;
        float pdf = 0.0f;
#endif

        // Torrence-Sparrow BRDF
        float3 specularBrdf = (D * G * F) / max(0.001f, 4 * NdV * NdL);
        throughput *= specularBrdf * NdL / max(0.001f, pdf) / max(0.001f, specProb);

#ifdef DEBUG_BUFFER
#     include "Debug/DebugBuffersMicrofacetSpec.hlsli"
#endif
    }
    else // Lambert
    {
        float3 L_s = RandHemisphereCosineSSpace(rngState);
        wi = ShadingToWorldSpace(L_s, T, B, Ns);

        float NdL = L_s.z;

        float pdf = NdL / PI;
        float3 diffuseBrdf = albedo * (1.0 - metalness);
        diffuseBrdf /= PI;
        throughput *= diffuseBrdf * NdL / max(0.001f, pdf) / max(0.001f, 1.0 - specProb);

#ifdef DEBUG_BUFFER
#     include "Debug/DebugBuffersMicrofacetDiff.hlsli"
#endif
    }
}

#endif