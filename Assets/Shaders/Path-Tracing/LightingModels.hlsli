#ifndef H_LIGHTING_MODELS_H
#define H_LIGHTING_MODELS_H

#include "Microfacet.hlsli"
#include "DualIncludes/HlslMath.h"

// Caller must flip normals + swap IoRs if exiting
float3 Refract(float3 wo, float3 Ns, float iorA, float iorB)
{
    float relIor = iorA / iorB;
    float cosI = -dot(wo, Ns);
    float sin2T = relIor * relIor * (1 - cosI * cosI);
    if (sin2T > 1) return float3(0,0,0); // Fully reflected, no refraction

    return normalize(wo * relIor + Ns * (relIor * cosI - sqrt(1 - sin2T)));
}

float3 Reflect(float3 wo, float3 Ns)
{
    return wo - 2 * dot(wo, Ns) * Ns;
}

#define IOR_AIR 1

struct GlassResponse
{
    float3 reflectDir;
    float reflectWeight;
    float3 refractDir;
};

// Calculated via the Fresnel equation
// Caller must flip normals + swap IoRs if exiting
float ReflectanceProportion(float3 wo, float3 Ns, float iorA, float iorB)
{
    float relIor = iorA / iorB;
    float cosI = -dot(wo, Ns);
    if (cosI <= 0) return 1; // Bad input

    float sin2T = relIor * relIor * (1 - cosI * cosI);
    if (sin2T >= 1) return 1; // Fully reflected

    float cosT = sqrt(1 - sin2T);
    float denomPerp = iorA * cosI + iorB * cosT;
    float denomPara = iorA * cosI + iorB * cosT; // iorA, iorB or iorB, iorA ?????

    if (min(denomPerp, denomPara) < 1E-8) return 1;

    float rPerp = (iorA * cosI - iorB * cosT) / denomPerp;
    float rPara = (iorB * cosI - iorA * cosT) / denomPara;

    return 0.5f * (rPerp * rPerp + rPara * rPara);
}

GlassResponse CalcReflectRefract(float3 wo, float3 Ns, float iorA, float iorB)
{
    GlassResponse res;
    res.reflectDir = Reflect(wo, Ns);
    res.reflectWeight = ReflectanceProportion(wo, Ns, iorA, iorB);
    res.refractDir = Refract(wo, Ns, iorA, iorB);
    return res;
}

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
    throughput *= albedo;

    float3 diffuseDir = normalize(Ns + RandDirectionSphere(rngState));
    wi = diffuseDir;
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

float3 NormalizeSafe(float3 N, float3 fallback)
{
    return length(N) == 0 ? fallback : normalize(N);
}

// https://backend.orbit.dtu.dk/ws/files/126824972/onb_frisvad_jgt2012_v2.pdf
void BuildBasisFrisvad(float3 N, out float3 T, out float3 B)
{
    if (N.z < -0.999999f)
    {
        T = float3(0, -1, 0);
        B = float3(-1, 0, 0);
        return;
    }

    float a = 1.0 / (1.0 + N.z);
    float b = -N.x * N.y * a;
    T = float3(1.0 - N.x * N.x * a, b, -N.x);
    B = float3(b, 1.0 - N.y * N.y * a, -N.y);
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
        float a2 = alpha * alpha;
        float3 H_s = SampleH_GGX(a2, rngState);
#elif defined(NDF_TYPE_BECKMANN)
        float alpha = RoughnessToAlpha_Beckmann(roughness);
        float a2 = alpha * alpha;
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
        float G = G_SmithGGX(NdL, NdV, a2);
        float pdf = Pdf_GGX(D, NdH, VdH);
        //float pdf = Pdf_General(D, G1_GGX(NdV, a2), V_s, H_s, pdfSampleVisibleArea);
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
        float3 L_s = RandHemisphereCosine(rngState, N_s);
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