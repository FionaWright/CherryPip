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