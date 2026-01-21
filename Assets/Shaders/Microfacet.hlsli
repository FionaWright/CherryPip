#ifndef H_MICROFACET_H
#define H_MICROFACET_H

#include "Rand01.hlsli"

float D_GGX(float NdH, float a2)
{
    float denominator = (NdH * NdH * (a2 - 1.0f) + 1.0f);
    return a2 / max(0.001f, PI * denominator * denominator);
}

float3 F_Schlick(float VdH, float3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - VdH, 5);
}

float G_GGX(float NdX, float a2)
{
    float denom = NdX + sqrt(a2 + (1-a2) * NdX * NdX);
    return 2 * NdX / max(0.001f, denom);
}

float G_Smith(float NdL, float NdV, float a2)
{
    return G_GGX(NdL, a2) * G_GGX(NdV, a2);
}

// Schlick-GGX Approximation. Faster but less accurate
float G_SmithFast(float NdL, float NdV, float roughness)
{
    float r = roughness + 1;
    float k = (r * r) / 8.0f;

    float ggxL = NdL / (NdL * (1.0f - k) + k);
    float ggxV = NdV / (NdV * (1.0f - k) + k);

    return ggxL * ggxV;
}

float3 SampleGGX_Classic(float3 N, float3 V, float a2, inout uint rngState)
{
    float3 T, B;
    T = any_perpendicular(N);
    B = cross(N, T);

    float r1 = PcgRand01(rngState);
    float r2 = PcgRand01(rngState);

    float phi = 2.0 * PI * r1;
    float cosTheta = sqrt((1.0 - r2) / (1.0 + (a2 - 1.0) * r2));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    float3 Ht = normalize(float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta));

    float3 H = normalize(Ht.x * T + Ht.y * B + Ht.z * N);
    return H;
}

float PdfGGX_Classic(float NdH, float VdH, float a2)
{
    float D = D_GGX(NdH, a2);
    return D * NdH / (4.0f * max(0.001f, VdH));
}

float PdfGGX_VNDF(float NdH, float VdH, float NdV, float a2)
{
    float D = D_GGX(NdH, a2);
    float G = G_GGX(NdV, a2);
    return G * D * VdH / max(0.001f, NdV);
}

#endif