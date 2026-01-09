float3 Refract(float3 wo, float3 Ns, float iorA, float iorB)
{
    float relIor = iorA / iorB;
    float cosAngleIn = -dot(wo, Ns);
    float sinSqrAngleRefraction = relIor * relIor * (1 - cosAngleIn * cosAngleIn);
    if (sinSqrAngleRefraction > 1) return 0; // Fully reflected, no refraction

    return wo * relIor + Ns * (relIor * cosAngleIn - sqrt(1 - sinSqrAngleRefraction));
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
float ReflectanceProportion(float3 wo, float3 Ns, float iorA, float iorB)
{
    float relIor = iorA / iorB;
    float cosAngleIn = -dot(wo, Ns);
    float sinSqrAngleRefraction = relIor * relIor * (1 - cosAngleIn * cosAngleIn);
    if (sinSqrAngleRefraction >= 1) return 1; // Fully reflected

    float cosAngleOfRefraction = sqrt(1 - sinSqrAngleRefraction);
    float denominatorPerpendicular = iorA * cosAngleIn + iorB * cosAngleOfRefraction;
    float denominatorParallel = iorA * cosAngleIn + iorB * cosAngleOfRefraction;

    if (min(denominatorPerpendicular, denominatorParallel) < 1E-8) return 1;

    // Perpendicular polarization
    float rPerpendicular = (iorA * cosAngleIn - iorB * cosAngleOfRefraction) / denominatorPerpendicular;
    rPerpendicular *= rPerpendicular;

    // Parallel polarization
    float rParallel = (iorB * cosAngleIn - iorA * cosAngleOfRefraction) / denominatorParallel;
    rParallel *= rParallel;

    // Return the average of the perpendicular and parallel polarizations
    return (rPerpendicular + rParallel) / 2;
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
    float3 brdf,
    out float3 wi)
{
    Lo += throughput * Li;
    throughput *= brdf;

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
    float3 brdf,
    float3 wo,
    out float3 wi)
{
    bool isDiffuse = diffuseProbability >= PcgRand01(rngState);

    Lo += throughput * Li;
    throughput *= lerp(float3(1, 1, 1), brdf, isDiffuse);

    float3 diffuseDir = normalize(Ns + RandDirectionSphere(rngState));
    float3 specularDir = wo - 2 * dot(wo, Ns) * Ns; // TODO: Replace with Reflect/2
    wi = lerp(specularDir, diffuseDir, roughness * isDiffuse);
}

void Model_Glass(
    inout uint rngState,
    inout float3 Lo,
    inout float3 throughput,
    PtMaterialData mat,
    bool isBackface,
    float3 Ns,
    float3 Li,
    float3 brdf,
    float3 wo,
    out float3 wi)
{
    //Lo += throughput * Li;
    if (isBackface)
        throughput *= brdf; // TODO: Absorb light as it enters object

    float iorCurrent = isBackface ? mat.IoR : IOR_AIR;
    float iorNext = isBackface ? IOR_AIR : mat.IoR;
    GlassResponse res = CalcReflectRefract(wo, Ns, iorCurrent, iorNext);

    float3 diffuseDir = normalize(Ns + RandDirectionSphere(rngState));
    res.reflectDir = normalize(lerp(res.reflectDir, diffuseDir, mat.DiffuseProbability));
    res.refractDir = normalize(lerp(res.refractDir, -diffuseDir, mat.Roughness));

    bool reflect = PcgRand01(rngState) <= res.reflectWeight;
    wi = reflect ? res.reflectDir : res.refractDir;
}