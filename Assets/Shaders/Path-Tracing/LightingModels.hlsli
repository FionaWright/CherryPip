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
    float3 specularDir = wo - 2 * dot(wo, Ns) * Ns;
    wi = lerp(specularDir, diffuseDir, roughness * isDiffuse);
}