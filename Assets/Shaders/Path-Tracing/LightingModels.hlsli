#include "Microfacet.hlsli"

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
    inout float3 Lo,
    inout float3 throughput,
    float3 Ns,
    float3 Li,
    float3 albedo,
    out float3 wi)
{
    Lo += throughput * Li;
    throughput *= albedo;

    float3 diffuseDir = normalize(Ns + RandDirectionSphere(rngState));
    wi = diffuseDir;
}

void Model_Glossy(
    inout uint rngState,
    inout float3 Lo,
    inout float3 throughput,
    float diffuseProbability,
    float roughness,
    float3 Ns,
    float3 Li,
    float3 albedo,
    float3 wo,
    out float3 wi)
{
    bool isDiffuse = diffuseProbability >= PcgRand01(rngState);

    Lo += throughput * Li;
    throughput *= lerp(float3(1, 1, 1), albedo, isDiffuse);

    float3 diffuseDir = normalize(Ns + RandDirectionSphere(rngState));
    float3 specularDir = Reflect(wo, Ns);
    wi = lerp(specularDir, diffuseDir, roughness * isDiffuse);
}

void Model_Glass(
    inout uint rngState,
    inout float3 Lo,
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
    out float3 wi)
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
    GlassResponse res = CalcReflectRefract(wo, Ns, iorCurrent, iorNext);

    float3 diffuseDir = normalize(Ns + RandDirectionSphere(rngState));
    res.reflectDir = normalize(lerp(res.reflectDir, diffuseDir, diffuseProbability)); // Why diffuseprobability and not roughness?
    res.refractDir = normalize(lerp(res.refractDir, -diffuseDir, roughness));

    bool reflect = PcgRand01(rngState) <= res.reflectWeight;
    wi = reflect ? res.reflectDir : res.refractDir;

    throughput *= reflect ? res.reflectWeight : (1.0 - res.reflectWeight);
}

float3 NormalizeSafe(float3 N, float3 fallback)
{
    return length(N) == 0 ? fallback : normalize(N);
}

// Next:
// Read this: https://medium.com/@Ksatese/advanced-ray-tracer-part-6-f7978842081f
// And this: https://henryzxu.github.io/pathtracing-p2/
// Then read PBRTs chapter on microfacet materials
// Then this whitepaper: https://dl.acm.org/doi/epdf/10.1145/3130800.3130806
// Finally find a repo with code I can look at and compare
// If still not working then park it and do importance sampling first maybe

// G_Smith is def broken. Check equation. Not sure if G_SmithFast is correct either but maybe
// Still unsure what specProb should be
// Fix hot reloading and add debug buffer inside specular for faster debugging
// Fresnel term doesn't quite match raster either but could be just more physically accurate (different wi)

// CHALLENGE: Fix it without GPT or help

void Model_GgxSmithSchlick(
    inout uint rngState,
    inout float3 Lo,
    inout float3 throughput,
    float roughness,
    float metalness,
    float3 Ns,
    float3 Li,
    float3 albedo,
    float3 wo,        // V
    out float3 wi     // L
#ifdef DEBUG_BUFFER
    , out float3 debug
    , out bool hasDebugOutput
#endif
)
{
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metalness);

    float specProb = 1.00f; // TODO once specular is fixed
    bool sampleSpecular = PcgRand01(rngState) < specProb;

    Lo += throughput * Li; // Wrong?

    if (sampleSpecular)
    {
        float alpha = roughness * roughness;
        float a2 = alpha * alpha;

        wi = SampleGGX_Classic(Ns, wo, a2, rngState);
        float3 H = NormalizeSafe(wo + wi, Ns);

        float NdL = dot(Ns, wi);
        float NdV = dot(Ns, wo);
        float NdH = dot(Ns, H);
        float VdH = dot(H, wo);

        float D = D_GGX(NdH, a2);
        float3 F = F_Schlick(VdH, F0);
        //float G = G_Smith(NdL, NdV, a2); // Wrong
        float G = G_SmithFast(NdL, NdV, roughness);

        float pdf = PdfGGX_Classic(NdH, VdH, a2);
        float3 specularBrdf = (D * G * F) / max(0.001f, 4 * NdV * NdL);
        throughput *= specularBrdf * NdL / max(0.001f, pdf) / max(0.001f, specProb);

#ifdef DEBUG_BUFFER
#     include "Debug/DebugBuffersMicrofacet.hlsli"
#endif
    }
    else // The diffuse is correct! Issue is in specular
    {
        wi = RandHemisphereCosine(rngState, Ns);

        float NdL = dot(wi, Ns);

        float pdf = NdL / PI;
        float3 diffuseBrdf = albedo * (1.0 - metalness);
        diffuseBrdf /= PI;
        throughput *= diffuseBrdf * NdL / pdf / max(0.001f, 1.0 - specProb);
    }
}