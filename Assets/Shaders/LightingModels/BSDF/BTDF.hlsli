#ifndef H_BTDF_H
#define H_BTDF_H

void BTDF(
    inout RngInfo rngInfo,
    inout float3 throughput,
    out float3 L_s,

    float roughness,
    float3 albedo,
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

    if (!entering)
        throughput *= exp(-sigmaA * hitDist);

    MicrofacetModel mm;
    InitializeMM(mm, roughness, rngInfo, V_s);
    //if (cAnisotropyEnabled)
    //    InitializeMMAniso(mm, T, B, N, anisoDirAndStrength);

    float3 H_s = normalize(mm.Sample(u1, u2));
    L_s = Refract(-V_s, H_s, nCurrent, nNext);

    float VdH = dot(H_s, V_s);

    float3 F = Dielectric_Unpolarized(nCurrent, nNext, abs(VdH));

    float D = mm.D(H_s);
    float G = mm.G2(L_s, V_s);
    float pdf = mm.PDF(D, H_s, V_s);
    //throughput *= D * F * G / max(0.001f, pdf);

    float eta = nCurrent / nNext;
    float eta2 = eta * eta;
    throughput *= eta2 * albedo;
}

#endif