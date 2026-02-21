void Model_Glass(
    inout RngInfo rngInfo,
    inout float3 throughput,
    float diffuseProbability,
    float roughness,
    bool entering,
    float hitDist,
    float ior,
    float3 sigmaA,
    float3 Ns,
    float3 Li,
    float3 albedo,
    float3 wo,
    out float3 wi,
    out float3 L_sample)
{
    if (!entering)
    {
        throughput *= exp(-sigmaA * hitDist);
    }

    float iorCurrent = entering ? IOR_AIR : ior;
    float iorNext = entering ? ior : IOR_AIR;
    GlassResponse res = CalcReflectRefract(-wo, Ns, iorCurrent, iorNext);

    float3 T, B;
    BuildBasisFrisvad(Ns, T, B);

    float u1 = Rand01_Bounce(DIM_D_BSDF_U1, rngInfo);
    float u2 = Rand01_Bounce(DIM_D_BSDF_U2, rngInfo);

    float3 diffuseDir = RandHemisphereUniformWorld(u1, u2, T, B, Ns);
    res.reflectDir = normalize(lerp(res.reflectDir, diffuseDir, diffuseProbability)); // Why diffuseprobability and not roughness?
    res.refractDir = normalize(lerp(res.refractDir, -diffuseDir, roughness));

    float rSpecProb = Rand01_Bounce(DIM_D_SPECULAR_PROB, rngInfo);
    bool reflect = rSpecProb <= res.reflectWeight;
    wi = reflect ? res.reflectDir : res.refractDir;

    if (!reflect)
        throughput *= albedo;

    L_sample = 0;
    throughput *= reflect ? res.reflectWeight : (1.0 - res.reflectWeight);
}