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

    float3 H_s = normalize(mm.Sample(u1, u2));
    L_s = Refract(-V_s, H_s, nCurrent, nNext);

    //float3 diffuseDir = RandHemisphereUniformSSpace(u1, u2);
    //L_s = normalize(lerp(L_s, -diffuseDir, roughness));

    float VdH = dot(H_s, V_s);
    float LdH = dot(L_s, H_s);

    float F = Dielectric_Unpolarized(nCurrent, nNext, abs(VdH));

    float denom = nCurrent * VdH + nNext * LdH;
    denom = denom * denom;
    float factor = abs(VdH * LdH) / max(1e-6f, denom);

    float D = mm.D(H_s);
    float G = mm.G2(L_s, V_s);
    float pdf = mm.PDF(D, H_s, V_s);
    //pdf *= abs(LdH) / max(1e-6f, denom);

    float eta = nCurrent / nNext;
    float eta2 = eta * eta;

    throughput *= (1 - F);
    //throughput *= D * G;
    //throughput /= max(1e-6f, pdf);
    //throughput *= factor;
    throughput *= eta2 * albedo;
}

#endif