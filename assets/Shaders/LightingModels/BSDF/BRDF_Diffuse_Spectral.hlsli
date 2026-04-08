#ifndef H_BRDF_DIFFUSE_SPECTRAL_H
#define H_BRDF_DIFFUSE_SPECTRAL_H

void BRDF_Diffuse_Spectral(
    inout RngInfo rngInfo,
    inout SpectralValue throughput,
    out float3 L_s,

    float metalness,
    SpectralValue albedo,

    inout float3 debug,
    inout bool hasDebugOutput)
{
    float u1 = Rand01_Bounce(DIM_D_BSDF_U1, rngInfo);
    float u2 = Rand01_Bounce(DIM_D_BSDF_U2, rngInfo);

    float pdf;
    SpectralValue diffuseBrdf;
    SpectralValue E;
    if (cImportanceSamplingEnabled)
    {
        L_s = RandHemisphereCosineSSpace(u1, u2);

        float NdL = SSpaceCosTheta(L_s);
        pdf = NdL / PI;
        diffuseBrdf = Mul(albedo, (1.0 - metalness) / PI); // Left in for debug view
        // E = diffuseBrdf * NdL / pdf
        E = Mul(albedo, (1.0 - metalness)); // Terms cancel out
    }
    else
    {
        L_s = RandHemisphereUniformSSpace(u1, u2);

        float NdL = SSpaceCosTheta(L_s);
        pdf = 1.0f / (2.0f * PI);
        diffuseBrdf = Mul(albedo, (1.0 - metalness) / PI);
        E = Mul(diffuseBrdf, NdL / max(0.001f, pdf));
    }

    throughput.Mul(E);

//#ifdef DEBUG_PT_INFO_OUTPUT
//#     include "Debug/DebugInfoOutputBRDFDiff.hlsli"
//#endif
}

#endif