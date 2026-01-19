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

float D_GGX(float NdH, float a2)
{
    float denominator = (NdH * NdH * (a2 - 1.0f) + 1.0f);
    return a2 / max(0.001f, PI * denominator * denominator);
}

float3 F_Schlick(float VdH, float3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - VdH, 5);
}

float G_GGX(float NdX, float a2)
{
    float denom = NdX + sqrt(a2 + (1-a2) * NdX * NdX);
    return 2 * NdX / denom;
}

float G_Smith(float NdL, float NdV, float a2)
{
    return G_GGX(NdL, a2) * G_GGX(NdV, a2);
}

// Schlick-GGX Approximation. Faster but less accurate
float G_SmithFast(float NdL, float NdV, float roughness)
{
    float r = roughness + 1;
    float k = (r * r) / 8.0f;

    float ggxL = NdL / (NdL * (1.0f - k) + k);
    float ggxV = NdV / (NdV * (1.0f - k) + k);

    return ggxL * ggxV;
}

float3 SampleGGX_Classic(float3 N, float3 V, float a2, inout uint rngState)
{
    float3 T, B;
    T = any_perpendicular(N);
    B = cross(N, T);

    float r1 = PcgRand01(rngState);
    float r2 = PcgRand01(rngState);

    float phi = 2.0 * PI * r1;
    float cosTheta = sqrt((1.0 - r2) / (1.0 + (a2 - 1.0) * r2));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    float3 Ht = normalize(float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta));

    float3 H = normalize(Ht.x * T + Ht.y * B + Ht.z * N);
    return H;
}

float PdfGGX_Classic(float NdH, float VdH, float a2)
{
    float D = D_GGX(NdH, a2);
    return D * NdH / (4.0f * max(0.001f, VdH));
}

float PdfGGX_VNDF(float NdH, float VdH, float NdV, float a2)
{
    float D = D_GGX(NdH, a2);
    float G = G_GGX(NdV, a2);
    return G * D * VdH / max(0.001f, NdV);
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

// Move to HlslMaths.h
float Luminance(float3 color)
{
    return dot(color, float3(0.2126,0.7152,0.0722));
}

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
    out float3 wi)    // L
{
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metalness);
    float3 diffuseColor = albedo * (1.0 - metalness);

    float specProb = saturate(max(F0.r, max(F0.g, F0.b))); // This seems wrong
    specProb = saturate(Luminance(F0) + 0.5 * roughness);
    bool sampleSpecular = PcgRand01(rngState) < specProb;

    Lo += throughput * Li; // What do I do with you?

    if (sampleSpecular)
    {
        float alpha = roughness * roughness;
        float a2 = alpha * alpha;

        wi = SampleGGX_Classic(Ns, wo, a2, rngState);
        float3 H = NormalizeSafe(wo + wi, Ns);

        float NdL = dot(wi, Ns);
        float NdV = dot(wo, Ns);
        float NdH = dot(Ns, H);
        float VdH = dot(H, wo);

        float D = D_GGX(NdH, a2);
        float3 F = F_Schlick(VdH, F0);
        float G = G_Smith(NdL, NdV, a2);

        float pdf = PdfGGX_Classic(NdH, VdH, a2); // Must match D_GGX
        float3 brdf = (D * G * F) / (4 * NdV * NdL);

        throughput *= brdf * NdL / pdf / specProb;
    }
    else
    {
        wi = RandHemisphereCosine(rngState, Ns);
        //float NdL = dot(wi, Ns);
        //float pdf = NdL / PI;
        //float3 brdf = diffuseColor / PI;
        throughput *= diffuseColor / max(0.001f, 1.0 - specProb); // brdf and pdf cancel to leave diffuseColor
    }
}