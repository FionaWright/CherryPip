#ifndef H_MICROFACET_H
#define H_MICROFACET_H

#include "Rand01.hlsli"

float3 WorldToShadingSpace(float3 X, float3 T, float3 B, float3 N)
{
    return normalize(float3(dot(X, T), dot(X, B), dot(N, T)));
}

float3 ShadingToWorldSpace(float3 X, float3 T, float3 B, float3 N)
{
    return normalize(X.x * T + X.y * B + X.z * N);
}

float SSpaceCosTheta(float3 X) { return X.z; }
float SSpaceCos2Theta(float3 X) { return X.z * X.z; }
float SSpaceSin2Theta(float3 X) { return max(0.0f, 1.0f - SSpaceCos2Theta(X)); }
float SSpaceSinTheta(float3 X) { return X.z; }
float SSpaceTanTheta(float3 X) { return SSpaceSinTheta(X) / SSpaceCosTheta(X); }
float SSpaceTan2Theta(float3 X) { return SSpaceSin2Theta(X) / SSpaceCos2Theta(X); }

float SSpaceCosPhi(float3 X)
{
    float sinT = SSpaceSinTheta(X);
    return sinT == 0 ? 1 : clamp(X.x / sinT, -1, 1);
}
float SSpaceSinPhi(float3 X)
{
    float sinT = SSpaceSinTheta(X);
    return sinT == 0 ? 1 : clamp(X.y / sinT, -1, 1);
}
float SSpaceCosDeltaPhi(float3 A, float3 B)
{
    return clamp((A.x * B.x + A.y * B.y) /
                 sqrt((A.x * A.x + A.y * A.y) *
                    (B.x * B.x + B.y * B.y)), -1, 1);
}


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

float3 SampleGGX_Classic(float a2, inout uint rngState)
{
    float r1 = PcgRand01(rngState);
    float r2 = PcgRand01(rngState);

    float phi = 2.0 * PI * r1;
    float cosTheta = sqrt((1.0 - r2) / (1.0 + (a2 - 1.0) * r2));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    float3 L_s = normalize(float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta));
    return L_s;
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