#ifndef H_BRDF_DIFFUSE_H
#define H_BRDF_DIFFUSE_H

void BRDF_Diffuse(
    inout RngInfo rngInfo,
    inout float3 throughput,
    out float3 L_s,

    float metalness,
    float3 albedo,

    inout float3 debug,
    inout bool hasDebugOutput)
{
    float u1 = Rand01_Bounce(DIM_D_BSDF_U1, rngInfo);
    float u2 = Rand01_Bounce(DIM_D_BSDF_U2, rngInfo);

    float pdf;
    float3 diffuseBrdf;
    float3 E;
    if (cImportanceSamplingEnabled)
    {
        L_s = RandHemisphereCosineSSpace(u1, u2);

        float NdL = SSpaceCosTheta(L_s);
        pdf = NdL / PI;
        diffuseBrdf = albedo * (1.0 - metalness) / PI; // Left in for debug view
        // E = diffuseBrdf * NdL / pdf
        E = albedo * (1.0 - metalness); // Terms cancel out
    }
    else
    {
        L_s = RandHemisphereUniformSSpace(u1, u2);

        float NdL = SSpaceCosTheta(L_s);
        pdf = 1.0f / (2.0f * PI);
        diffuseBrdf = albedo * (1.0 - metalness) / PI;
        E = diffuseBrdf * NdL / max(0.001f, pdf);
    }

    throughput *= E;

#ifdef DEBUG_PT_INFO_OUTPUT
#     include "Debug/DebugInfoOutputBRDFDiff.hlsli"
#endif
}

#endif