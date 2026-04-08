#ifndef H_MODEL_LAMBERTIAN_H
#define H_MODEL_LAMBERTIAN_H

void Model_LambertionDiffuse(
    inout RngInfo rngInfo,
    inout float3 throughput,
    float3 Ns,
    float3 Li,
    float3 albedo,
    out float3 wi,
    out float3 L_sample)
{
    L_sample = throughput * Li;

    float u1 = Rand01_Bounce(DIM_D_BSDF_U1, rngInfo);
    float u2 = Rand01_Bounce(DIM_D_BSDF_U2, rngInfo);

    ShadingFrame sframe = CreateShadingFrame(Ns);

    if (cImportanceSamplingEnabled)
    {
        wi = RandHemisphereCosineWorld(u1, u2, sframe);
        // diffuseBrdf = albedo / PI
        // pdf = NdL / PI
        // throughput *= diffuseBrdf * NdL / pdf
        throughput *= albedo; // Terms cancel out
        return;
    }

    wi = RandHemisphereUniformWorld(u1, u2, sframe);
    float NdL = saturate(dot(Ns, wi));

    float3 diffuseBrdf = albedo / PI;
    float pdf = 1.0f / (2.0f * PI);
    throughput *= diffuseBrdf * NdL / max(0.001f, pdf);
}

#endif