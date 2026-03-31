#ifndef H_MODEL_LAMBERTION_SPECTRAL_H
#define H_MODEL_LAMBERTION_SPECTRAL_H

void Model_LambertionDiffuse_Spectral(
    inout RngInfo rngInfo,
    inout SpectralValue throughput,
    float3 Ns,
    SpectralValue Li,
    SpectralValue albedo,
    out float3 wi,
    out SpectralValue L_sample)
{
    L_sample = Mul(throughput, Li);

    float u1 = Rand01_Bounce(DIM_D_BSDF_U1, rngInfo);
    float u2 = Rand01_Bounce(DIM_D_BSDF_U2, rngInfo);

    ShadingFrame sframe = CreateShadingFrame(Ns);

    if (cImportanceSamplingEnabled)
    {
        wi = RandHemisphereCosineWorld(u1, u2, sframe);
        // diffuseBrdf = albedo / PI
        // pdf = NdL / PI
        // throughput *= diffuseBrdf * NdL / pdf
        throughput.Mul(albedo); // Terms cancel out
        return;
    }

    wi = RandHemisphereUniformWorld(u1, u2, sframe);
    float NdL = saturate(dot(Ns, wi));

    SpectralValue diffuseBrdf = Div(albedo, PI);
    float pdf = 1.0f / (2.0f * PI);
    throughput.Mul(Mul(diffuseBrdf, NdL / max(0.001f, pdf)));
}

#endif