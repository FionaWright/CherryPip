#ifndef H_MICROFACET_H
#define H_MICROFACET_H

#include "Rand01.hlsli"
#include "Path-Tracing/MathUtils.hlsli"

// https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models

// ================================
//  Shading Space Math Utils
// ================================

float3 WorldToShadingSpace(float3 X, float3 T, float3 B, float3 N)
{
    return normalize(float3(dot(X, T), dot(X, B), dot(X, N)));
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
float SSpaceCos2Phi(float3 X) { return SSpaceCosPhi(X) * SSpaceCosPhi(X); }
float SSpaceSin2Phi(float3 X) { return SSpaceSinPhi(X) * SSpaceSinPhi(X); }

float SSpaceCosDeltaPhi(float3 A, float3 B)
{
    return clamp((A.x * B.x + A.y * B.y) /
                 sqrt((A.x * A.x + A.y * A.y) *
                    (B.x * B.x + B.y * B.y)), -1, 1);
}

// ================================
//  Roughness To Alpha
// ================================

float RoughnessToAlpha_GGX(float roughness)
{
    return roughness * roughness;
}

// Heuristic to have beckmann visually match GGX for the same roughness value
float RoughnessToAlpha_Beckmann(float roughness)
{
    roughness = max(1e-3f, roughness);
    float x = log(roughness);
    return 1.62142f + 0.819955f * x + 0.1734f * x * x +
           0.0171201f * x * x * x + 0.000640711f * x * x * x * x;
}

// ================================
//  Normal Distribution Functions
// ================================

// GGX == Trowbridge-Reitz
float D_GGX(float NdH, float a2)
{
    float denominator = (NdH * NdH * (a2 - 1.0f) + 1.0f);
    return a2 / max(0.001f, PI * denominator * denominator);
}

float D_Beckmann(float3 H, float a2)
{
    float tan2T = SSpaceTan2Theta(H);
    if (IsNaN(1/tan2T)) return 0; // Is infinite

    float cos4T = SSpaceCos2Theta(H) * SSpaceCos2Theta(H);
    float k3 = PI * a2 * cos4T;
    return exp(-tan2T / a2) / k3;
}

float D_BeckmannAniso(float3 H, float alphaX, float alphaY)
{
    float tan2T = SSpaceTan2Theta(H);
    if (IsNaN(1/tan2T)) return 0; // Is infinite

    float cos4T = SSpaceCos2Theta(H) * SSpaceCos2Theta(H);
    float k1 = SSpaceCos2Phi(H) / (alphaX * alphaX);
    float k2 = SSpaceSin2Phi(H) / (alphaY * alphaY);
    float k3 = PI * alphaX * alphaY * cos4T;
    return exp(-tan2T * (k1 + k2)) / k3;
}

// ================================
//  Fresnel Functions
// ================================

float3 F_Schlick(float VdH, float3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - VdH, 5);
}

// ================================
//  Geometry Masking Functions
// ================================

// Used for anisotropic microfacet models
// Compute alpha from alphaX and alphaY and use it in below functions
float AnisoAlphaXyToMaskingAlpha(float3 dirOfInterest, float alphaX, float alphaY)
{
    float k1 = SSpaceCos2Phi(dirOfInterest) * alphaX * alphaX;
    float k2 = SSpaceSin2Phi(dirOfInterest) * alphaY * alphaY;
    return sqrt(k1 + k2);
}

float G1_GGX(float NdX, float a2)
{
    float denom = NdX + sqrt(a2 + (1-a2) * NdX * NdX);
    return saturate(2 * NdX / max(0.001f, denom));
}

float G_SmithGGX(float NdL, float NdV, float a2)
{
    return G1_GGX(NdL, a2) * G1_GGX(NdV, a2);
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

// Implicit Height-Correlated / UE4 Heitz Style Beckmann
// TODO: Get beckmann smith
float Lambda_Beckmann(float3 W, float alpha)
{
    float absTanT = abs(SSpaceTanTheta(W));
    if (IsNaN(1/absTanT)) return 0; // Is infinite

    float a = 1 / (alpha * absTanT);
    if (a >= 1.6f) return 0;

    return (1 - 1.259f * a + 0.396f * a * a) /
           (3.535f * a + 2.181f * a * a);
}

float G1_Beckmann(float3 W, float alpha)
{
    return 1.0f / (1.0f + Lambda_Beckmann(W, alpha));
}

float G_Beckmann(float3 V, float3 L, float alpha)
{
    return 1.0f / (1.0f + Lambda_Beckmann(V, alpha) + Lambda_Beckmann(L, alpha));
}

// ================================
//  Direction Sampling Functions
// ================================

float3 SampleH_GGX(float a2, inout uint rngState)
{
    float r1 = PcgRand01(rngState);
    float r2 = PcgRand01(rngState);

    float phi = 2.0 * PI * r1;
    float cosTheta = sqrt((1.0 - r2) / (1.0 + (a2 - 1.0) * r2));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    float3 H_s = normalize(float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta));
    return H_s;
}

float3 SampleH_Beckmann(float alpha, inout uint rngState)
{
    float r1 = PcgRand01(rngState);
    float r2 = PcgRand01(rngState);

    float tan2Theta = -alpha * alpha * log(1.0 - r1);
    float cosTheta = 1.0 / sqrt(1.0 + tan2Theta);
    float sinTheta = sqrt(max(0.0, 1.0 - cosTheta * cosTheta));

    float phi = 2.0 * PI * r2;

    float3 H_s = normalize(float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta));
    return H_s;
}

// ================================
//  Probability Density Functions
// ================================

float Pdf_GGX(float D, float NdH, float VdH)
{
    return D * NdH / (4.0f * max(0.001f, VdH));
}

float Pdf_General(float D, float G1v, float3 V, float3 H, bool sampleVisibleArea)
{
    if (!sampleVisibleArea)
        return D * abs(SSpaceCosTheta(H));

    return D * G1v * abs(dot(V, H)) / abs(SSpaceCosTheta(V));
}

#endif