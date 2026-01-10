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
    float sin2T = relIor * relIor * (1 - cosI * cosI);
    if (sin2T >= 1) return 1; // Fully reflected

    float cosT = sqrt(1 - sin2T);
    float denomPerp = iorA * cosI + iorB * cosT;
    float denomPara = iorB * cosI + iorA * cosT;

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
    float3 specularDir = Reflect(wo, Ns);
    wi = lerp(specularDir, diffuseDir, roughness * isDiffuse);
}

void Model_Glass(
    inout uint rngState,
    inout float3 Lo,
    inout float3 throughput,
    PtMaterialData mat,
    bool isBackface,
    float hitDist,
    float3 Ns,
    float3 Li,
    float3 brdf,
    float3 wo,
    out float3 wi)
{
    bool entering = !isBackface;

    if (!entering)
        throughput *= exp(-0 * hitDist); // TODO: sigma_a

    float iorCurrent = entering ? IOR_AIR : mat.IoR;
    float iorNext = entering ? mat.IoR : IOR_AIR;
    GlassResponse res = CalcReflectRefract(wo, Ns, iorCurrent, iorNext);

    float3 diffuseDir = normalize(Ns + RandDirectionSphere(rngState));
    res.reflectDir = normalize(lerp(res.reflectDir, diffuseDir, mat.DiffuseProbability));
    res.refractDir = normalize(lerp(res.refractDir, diffuseDir, mat.Roughness));

    bool reflect = PcgRand01(rngState) <= res.reflectWeight;
    wi = reflect ? res.reflectDir : res.refractDir;

    // TODO: Weight throughput by reflect pdf
    //throughput *= reflect ? res.reflectWeight : (1.0 - res.reflectWeight);
}