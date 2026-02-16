#ifndef H_LIGHTING_MODELS_H
#define H_LIGHTING_MODELS_H

#include "MicrofacetModels/AllModels.hlsli"
#include "DualIncludes/HlslMath.h"
#include "MathUtils.hlsli"

void Model_LambertionDiffuse(
    inout RngInfo rngInfo,
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

    float u1 = Rand01_Bounce(DIM_D_BSDF_U1, rngInfo);
    float u2 = Rand01_Bounce(DIM_D_BSDF_U2, rngInfo);

#ifdef IMPORTANCE_SAMPLING
    wi = RandHemisphereCosineWorld(u1, u2, T, B, Ns);
    // diffuseBrdf = albedo / PI
    // pdf = NdL / PI
    // throughput *= diffuseBrdf * NdL / pdf
    throughput *= albedo; // Terms cancel out
#else
    wi = RandHemisphereUniformWorld(u1, u2, T, B, Ns);
    float NdL = saturate(dot(Ns, wi));
    float3 diffuseBrdf = albedo / PI;
    float pdf = 1.0f / (2.0f * PI);
    throughput *= diffuseBrdf * NdL / max(0.001f, pdf);
#endif
}

void Model_Glossy(
    inout RngInfo rngInfo,
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
    //float u1 = Rand01_Bounce(rngInfo.BounceBaseDimension + DIM_D_BSDF_U1, globalSampleIdx, rngState);
    //float u2 = Rand01_Bounce(rngInfo.BounceBaseDimension + DIM_D_BSDF_U2, globalSampleIdx, rngState);

    float rSpecProb = Rand01_Bounce(DIM_D_SPECULAR_PROB, rngInfo);
    bool isDiffuse = diffuseProbability >= rSpecProb;

    L_sample = throughput * Li;
    throughput *= lerp(float3(1, 1, 1), albedo, isDiffuse);

    float3 diffuseDir = normalize(Ns + RandDirectionSphere(rngInfo.IndependentRngState)); // TODO: Bad sampling
    float3 specularDir = Reflect(-wo, Ns);
    wi = lerp(specularDir, diffuseDir, roughness * isDiffuse);
}

void Model_Glass(
    inout RngInfo rngInfo,
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

    //float u1 = Rand01(rngInfo.BounceBaseDimension + DIM_D_BSDF_U1, globalSampleIdx, rngState);
    //float u2 = Rand01(rngInfo.BounceBaseDimension + DIM_D_BSDF_U2, globalSampleIdx, rngState);

    float3 diffuseDir = normalize(Ns + RandDirectionSphere(rngInfo.IndependentRngState)); // TODO: Bad sampling
    res.reflectDir = normalize(lerp(res.reflectDir, diffuseDir, diffuseProbability)); // Why diffuseprobability and not roughness?
    res.refractDir = normalize(lerp(res.refractDir, -diffuseDir, roughness));

    float rSpecProb = Rand01_Bounce(DIM_D_SPECULAR_PROB, rngInfo);
    bool reflect = rSpecProb <= res.reflectWeight;
    wi = reflect ? res.reflectDir : res.refractDir;

    L_sample = 0;
    throughput *= reflect ? res.reflectWeight : (1.0 - res.reflectWeight);
}

// TODO:
// 3. Get Aniso GGX working
// 2. Get VNDF GGX working (see also https://arxiv.org/pdf/2306.05044)
// 4. Get Beckmann working (Smith first, then lambda)
// 5. Get Aniso Beckmann working
// 6. Beckmann VNDF

// TODO: Aniso
// Get the barn lamp and strength test aniso models
// Read GLTF spec and copy code from there, don't delete any old code
// Get it working

// Avoid code breaking
#if !defined(NDF_TYPE_GGX) && !defined(NDF_TYPE_BECKMANN)
#define NDF_TYPE_GGX
#endif

void Model_Microfacet(
    inout RngInfo rngInfo,
    inout float3 throughput,
    float roughness,
    float metalness,
    float3 Ns,
    float3 Li,
    float3 albedo,
    float3 wo,        // V
    out float3 wi,    // L
    out float3 L_sample
#ifdef ANISOTROPY_ENABLED
    , float3 anisoDirAndStrength
#endif
#ifdef DEBUG_PT_INFO_OUTPUT
    , inout float3 debug
    , inout bool hasDebugOutput
#endif
)
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

    float specProb = clamp(Luminance(F_select), 0.05f, 0.95f); // kS

#if defined(DEBUG_FORCE_SPECULAR)
    specProb = 1.0f;
#elif defined(DEBUG_FORCE_DIFFUSE)
    specProb = 0.0f;
#endif

    float u1 = Rand01_Bounce(DIM_D_BSDF_U1, rngInfo);
    float u2 = Rand01_Bounce(DIM_D_BSDF_U2, rngInfo);
    float rSpecProb = Rand01_Bounce(DIM_D_SPECULAR_PROB, rngInfo);

    bool isSpecular = rSpecProb < specProb;

    if (isSpecular)
    {
        MicrofacetModel mm;
        InitializeMM(mm, roughness, rngInfo);
#ifdef ANISOTROPY_ENABLED
        InitializeMMAniso(mm, T, B, Ns, anisoDirAndStrength);
#endif

        float3 H_s = mm.Sample(u1, u2);
        float3 L_s = normalize(reflect(-V_s, H_s));
        wi = InvToDefinedSpace(L_s, T, B, Ns);

        if (L_s.z <= 0.0f) // Can I get rid of this? Only needed for aniso
            return;

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
        //throughput *= L_s.z < 0;

#ifdef DEBUG_PT_INFO_OUTPUT
#     include "Debug/DebugInfoOutputMicrofacetSpec.hlsli"
#endif
    }
    else // Lambert
    {
#ifdef IMPORTANCE_SAMPLING
        float3 L_s = RandHemisphereCosineSSpace(u1, u2);
        wi = InvToDefinedSpace(L_s, T, B, Ns);

        float NdL = SSpaceCosTheta(L_s);
        float3 pdf = NdL / PI;
        float3 diffuseBrdf = albedo * (1.0 - metalness) / PI; // Left in for debug view
        // E = diffuseBrdf * NdL / pdf
        float3 E = albedo * (1.0 - metalness); // Terms cancel out
#else
        float3 L_s = RandHemisphereUniformSSpace(u1, u2);
        wi = InvToDefinedSpace(L_s, T, B, Ns);

        float NdL = SSpaceCosTheta(L_s);
        float pdf = 1.0f / (2.0f * PI);
        float3 diffuseBrdf = albedo * (1.0 - metalness) / PI;
        float3 E = diffuseBrdf * NdL / max(0.001f, pdf);
#endif
        E /= max(0.001f, 1.0 - specProb);
        throughput *= E;

#ifdef DEBUG_PT_INFO_OUTPUT
#     include "Debug/DebugInfoOutputMicrofacetDiff.hlsli"
#endif
    }
}

#endif