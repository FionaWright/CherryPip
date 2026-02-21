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