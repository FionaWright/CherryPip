void Model_Glass_Spectral(
    inout RngInfo rngInfo,
    inout SpectralValue throughput,
    float diffuseProbability,
    float roughness,
    bool entering,
    float hitDist,
    float ior,
    float3 sigmaA,
    float3 Ns,
    SpectralValue Li,
    SpectralValue albedo,
    float lambda,
    float3 wo,
    out float3 wi,
    out SpectralValue L_sample)
{
    if (!entering)
    {
        SpectralValue sigmaASpectral;
#ifdef SINGLE_LAMBDA_RENDERING
        sigmaASpectral = RgbToSpectrumSample(sigmaA, eReflectance, lambda);
#else
        sigmaASpectral.InitFromRGB(sigmaA, eReflectance);
#endif
#ifdef SINGLE_LAMBDA_RENDERING // TODO: Make SpectralValue a struct with the same pattern as Spectrum maybe? Use polymorphism technique
        throughput *= exp(-sigmaASpectral * hitDist);
#else
        sigmaASpectral.Mul(-hitDist);
        sigmaASpectral.Exp();
        throughput.Mul(sigmaASpectral);
#endif
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

#ifdef SINGLE_LAMBDA_RENDERING // TODO: Make SpectralValue a struct with the same pattern as Spectrum maybe? Use polymorphism technique
    if (!reflect)
        throughput *= albedo;
#else
    if (!reflect)
        throughput.Mul(albedo);
#endif

    L_sample = BlackSpectralValue();
#ifdef SINGLE_LAMBDA_RENDERING // TODO: Make SpectralValue a struct with the same pattern as Spectrum maybe? Use polymorphism technique
    throughput *= reflect ? res.reflectWeight : (1.0 - res.reflectWeight);
#else
    throughput.Mul(reflect ? res.reflectWeight : (1.0 - res.reflectWeight));
#endif
}