#ifndef H_BTDF_SPECTRAL_H
#define H_BTDF_SPECTRAL_H

// https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf

void BTDF_Spectral(
    inout RngInfo rngInfo,
    inout SpectralValue throughput,
    SpectralContext ctx,
    out float3 L_s,

    float roughness,
    SpectralValue albedo,
    float3 V_s,
    float3 N_s,

    bool entering,
    float nCurrent,
    float nNext,
    float3 sigmaA,
    float hitDist,

    inout float3 debug,
    inout bool hasDebugOutput)
{
    float u1 = Rand01_Bounce(DIM_D_BSDF_U1, rngInfo);
    float u2 = Rand01_Bounce(DIM_D_BSDF_U2, rngInfo);

    float eta = nCurrent / nNext;
    float eta2 = eta * eta;

    if (!entering)
    {
        SpectralValue sigmaASpectral;
        sigmaASpectral.FromRGB(sigmaA, eReflectance, ctx);
        sigmaASpectral.Mul(-hitDist);
        throughput.Mul(Exp(sigmaASpectral));
    }

    MicrofacetModel mm;
    InitializeMM(mm, roughness, rngInfo, V_s);

    float3 H_s = normalize(mm.Sample(u1, u2));
    L_s = refract(-V_s, H_s, eta);

    float VdH = dot(H_s, V_s);
    float LdH = dot(L_s, H_s);
    float NdL = SSpaceCosTheta(L_s);
    float NdV = SSpaceCosTheta(V_s);
    float NdH = SSpaceCosTheta(H_s);

    float F = Fresnel_Dielectric_Unpolarized(nCurrent, nNext, abs(VdH));

    if (roughness < 0.001f)
    {
        throughput.Mul((1 - F) * eta);
    }
    else
    {
        throughput.Mul(1 - F);
        throughput.Mul(Mul(albedo, eta2));
    }
}

#endif