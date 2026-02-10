#ifndef H_LIGHTING_MODELS_H
#define H_LIGHTING_MODELS_H

#include "Microfacet.hlsli"
#include "DualIncludes/HlslMath.h"
#include "MathUtils.hlsli"

void Model_LambertionDiffuse(
    inout uint rngState,
    inout float3 throughput,
    uint rngBaseDimension,
    uint rngSampleIdx,
    float3 Ns,
    float3 Li,
    float3 albedo,
    out float3 wi,
    out float3 L_sample)
{
    L_sample = throughput * Li;

    float3 T, B;
    BuildBasisFrisvad(Ns, T, B);

    float u1 = Rand01(rngBaseDimension + DIM_D_BSDF_U1, rngSampleIdx, rngState);
    float u2 = Rand01(rngBaseDimension + DIM_D_BSDF_U2, rngSampleIdx, rngState);

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
    inout uint rngState,
    inout float3 throughput,
    uint rngBaseDimension,
    uint rngSampleIdx,
    float diffuseProbability,
    float roughness,
    float3 Ns,
    float3 Li,
    float3 albedo,
    float3 wo,
    out float3 wi,
    out float3 L_sample)
{
    //float u1 = Rand01(rngBaseDimension + DIM_D_BSDF_U1, rngSampleIdx, rngState);
    //float u2 = Rand01(rngBaseDimension + DIM_D_BSDF_U2, rngSampleIdx, rngState);

    float rSpecProb = Rand01(rngBaseDimension + DIM_D_SPECULAR_PROB, rngSampleIdx, rngState);
    bool isDiffuse = diffuseProbability >= rSpecProb;

    L_sample = throughput * Li;
    throughput *= lerp(float3(1, 1, 1), albedo, isDiffuse);

    float3 diffuseDir = normalize(Ns + RandDirectionSphere(rngState)); // TODO: Bad sampling
    float3 specularDir = Reflect(-wo, Ns);
    wi = lerp(specularDir, diffuseDir, roughness * isDiffuse);
}

void Model_Glass(
    inout uint rngState,
    inout float3 throughput,
    uint rngBaseDimension,
    uint rngSampleIdx,
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

    //float u1 = Rand01(rngBaseDimension + DIM_D_BSDF_U1, rngSampleIdx, rngState);
    //float u2 = Rand01(rngBaseDimension + DIM_D_BSDF_U2, rngSampleIdx, rngState);

    float3 diffuseDir = normalize(Ns + RandDirectionSphere(rngState)); // TODO: Bad sampling
    res.reflectDir = normalize(lerp(res.reflectDir, diffuseDir, diffuseProbability)); // Why diffuseprobability and not roughness?
    res.refractDir = normalize(lerp(res.refractDir, -diffuseDir, roughness));

    float rSpecProb = Rand01(rngBaseDimension + DIM_D_SPECULAR_PROB, rngSampleIdx, rngState);
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
    inout uint rngState,
    inout float3 throughput,
    uint rngBaseDimension,
    uint rngSampleIdx,
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
    // forall X in R^3, X.z = dot(N, X)
    float3 N_s = float3(0, 0, 1);
    float3 V_s = ToDefinedSpace(wo, T, B, Ns);

    float NdV = SSpaceCosTheta(V_s);

    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metalness);
    float3 F_select = F_Schlick(NdV, F0);

    float specProb = clamp(Luminance(F_select), 0.05f, 0.95f); // kS

#if defined(FORCE_SPECULAR)
    specProb = 1.0f;
#elif defined(FORCE_DIFFUSE)
    specProb = 0.0f;
#endif

    float u1 = Rand01(rngBaseDimension + DIM_D_BSDF_U1, rngSampleIdx, rngState);
    float u2 = Rand01(rngBaseDimension + DIM_D_BSDF_U2, rngSampleIdx, rngState);
    float rSpecProb = Rand01(rngBaseDimension + DIM_D_SPECULAR_PROB, rngSampleIdx, rngState);

    bool isSpecular = rSpecProb < specProb;

    if (isSpecular)
    {
#if defined(NDF_TYPE_GGX)
#    ifdef ANISOTROPY_ENABLED
        float alpha = RoughnessToAlpha_GGX(roughness);
        float2 alphaXY = AlphaToAnisoAlpha(alpha, anisoDirAndStrength.z);
        float alphaX = max(1e-3, alphaXY.x);
        float alphaY = max(1e-3, alphaXY.y);
        bool isAniso = abs(alphaX - alphaY) > 0.0001f;
        float3 H_s;
        if (isAniso)
        {
            float3 anisoT = float3(anisoDirAndStrength.xy, 0);
            float3 anisoB = float3(-anisoDirAndStrength.y, anisoDirAndStrength.x, 0);
            float3 anisoN = float3(0, 0, 1);
            H_s = SampleH_GGXAniso(alphaX, alphaY, anisoDirAndStrength.xy, anisoT, anisoB, anisoN, u1, u2);
        }
        else
            H_s = SampleH_GGX(alpha * alpha, u1, u2);
#    else
#        ifdef SAMPLE_VISIBLE_NORMALS
        float alpha = RoughnessToAlpha_GGX(roughness);
        float a2 = max(1e-6f, alpha * alpha);
        float u3 = Rand01(rngBaseDimension + DIM_D_BSDF_U3, rngSampleIdx, rngState);
        float3 H_s = SampleH_VCavity_VNDF(a2, V_s, u1, u2, u3);
#        else
        float alpha = RoughnessToAlpha_GGX(roughness);
        float a2 = max(1e-6f, alpha * alpha);
        float3 H_s = SampleH_GGX(a2, u1, u2);
#        endif
#    endif
#elif defined(NDF_TYPE_BECKMANN) // TODO
        float alpha = RoughnessToAlpha_Beckmann(roughness); // TODO: Walters Trick here?
        float a2 = max(1e-6f, alpha * alpha);
        float3 H_s = SampleH_Beckmann(a2, u1, u2);
#endif

        float3 L_s = normalize(reflect(-V_s, H_s));
        wi = ToDefinedSpace(L_s, T, B, Ns);

        if (L_s.z <= 0.0f) // Can I get rid of this? Only needed for aniso
            return;

        float NdL = SSpaceCosTheta(L_s);
        float NdH = SSpaceCosTheta(H_s);
        float VdH = dot(H_s, V_s);

        float3 F = F_Schlick(VdH, F0);

        float D, G, pdf;
#if defined(NDF_TYPE_GGX)
#    ifdef ANISOTROPY_ENABLED
        if (isAniso)
        {
            D = D_GGXAniso(H_s, alphaX, alphaY);
            //float G = G_SmithGGXAniso(L_s, V_s, alphaX, alphaY, anisoT, anisoB, anisoN);
            G = G_GGXAniso(L_s, V_s, alphaX, alphaY);
            pdf = Pdf_GGXAniso(D, NdH, VdH);
        }
        else
        {
            D = D_GGX(NdH, alpha * alpha);
            G = G_SmithGGX(NdL, NdV, alpha * alpha);
            pdf = Pdf_GGX(D, NdH, VdH);
        }
#    else
#        ifdef SAMPLE_VISIBLE_NORMALS
        D = D_GGX(NdH, a2); // Wrong
        G = G_VCavity(V_s, L_s, H_s);
        float Dv = D * G1_VCavity(NdV, a2) * max(0.0f, VdH) / NdV;
        pdf = Pdf_GGX_VNDF(Dv, VdH);
#        else
        D = D_GGX(NdH, a2);
        G = G_SmithGGX(NdL, NdV, a2);
        pdf = Pdf_GGX(D, NdH, VdH);
#        endif
#    endif
#elif defined(NDF_TYPE_BECKMANN) // TODO
        D = D_Beckmann(H_s, a2);
        G = G_Beckmann(V_s, L_s, alpha);
        //pdf = Pdf_General(D, G1_Beckmann(V_s, alpha), V_s, H_s, pdfSampleVisibleArea);
#endif

        // Torrence-Sparrow BRDF
        float3 specularBrdf = (D * G * F) / max(0.001f, 4 * NdV * NdL);
        throughput *= specularBrdf * NdL / max(0.001f, pdf) / max(0.001f, specProb);
        //throughput *= L_s.z < 0;

#ifdef DEBUG_BUFFER
#     include "Debug/DebugBuffersMicrofacetSpec.hlsli"
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

#ifdef DEBUG_BUFFER
#     include "Debug/DebugBuffersMicrofacetDiff.hlsli"
#endif
    }
}

#endif