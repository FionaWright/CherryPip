#ifndef H_MICROFACET_H
#define H_MICROFACET_H

#include "Random.h"
#include "Path-Tracing/MathUtils.hlsli"

// https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models

// ================================
//  Shading Space Math Utils
// ================================

float3 ToDefinedSpace(float3 X, float3 T, float3 B, float3 N)
{
    return normalize(float3(dot(X, T), dot(X, B), dot(X, N)));
}

float3 InvToDefinedSpace(float3 X, float3 T, float3 B, float3 N)
{
    return normalize(X.x * T + X.y * B + X.z * N);
}

float SSpaceCosTheta(float3 X) { return X.z; }
float SSpaceCos2Theta(float3 X) { return X.z * X.z; }
float SSpaceSin2Theta(float3 X) { return max(0.0f, 1.0f - SSpaceCos2Theta(X)); }
float SSpaceSinTheta(float3 X) { return X.z; }
float SSpaceTanTheta(float3 X) { return SSpaceSinTheta(X) / SSpaceCosTheta(X); }
float SSpaceTan2Theta(float3 X) { return SSpaceSin2Theta(X) / SSpaceCos2Theta(X); }
float SSpaceCotTheta(float3 X) { return SSpaceCosTheta(X) / SSpaceSinTheta(X); }

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
//  Alpha
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

// Returns alpha', a better alpha for beckmann distributions
float WaltersTrick(float alpha, float NdL)
{
    return 1.2f - 0.2f * sqrt(abs(NdL)) * alpha;
}

float2 AlphaToAnisoAlpha(float alpha, float anisotropyStrength)
{
    //float aspect = sqrt(1.0f - 0.9 * abs(anisotropyStrength));
    //return float2(alpha / aspect, alpha * aspect);

    float strength2 = anisotropyStrength * anisotropyStrength;
    return float2(lerp(alpha, 1.0f, strength2), alpha);
}

// Used for anisotropic microfacet models
// Compute alpha from alphaX and alphaY and use it in below functions
float AnisoAlphaToMaskingAlpha(float3 dirOfInterest, float alphaX, float alphaY)
{
    float k1 = SSpaceCos2Phi(dirOfInterest) * alphaX * alphaX;
    float k2 = SSpaceSin2Phi(dirOfInterest) * alphaY * alphaY;
    return sqrt(k1 + k2);
}

// ================================
//  Direction Sampling Functions
// ================================

float3 SampleH_GGX(float a2, float u1, float u2)
{
    float phi = 2.0 * PI * u1;
    float cosTheta = sqrt(max(0.0f, (1.0 - u2) / max(0.001f, 1.0 + (a2 - 1.0) * u2)));
    float sinTheta = sqrt(max(0.0f, 1.0 - cosTheta * cosTheta));

    float3 H_s = normalize(float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta));
    return H_s;
}

float3 SampleH_GGXAniso(float alphaX, float alphaY, float2 anisoDir, float3 T, float3 B, float3 N, float u1, float u2)
{
    float phi = atan(alphaY / alphaX * tan(2.0 * PI * u1));
    float sinPhi = sin(phi);
    float cosPhi = cos(phi);

    float alpha2 =
        1.0 / (cosPhi*cosPhi / (alphaX*alphaX) +
               sinPhi*sinPhi / (alphaY*alphaY));

    float cosTheta = sqrt((1.0 - u2) / (1.0 + (alpha2 - 1.0) * u2));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

    return normalize(sinTheta * cosPhi * T +
        sinTheta * sinPhi * B +
        cosTheta * N);
}

// https://inria.hal.science/hal-00996995v2
// VNDF only works with mathematically well-defined G1 models (V-Cavity + Smith)
// Ignore this one, get Smith_VNDF working first imo
float3 SampleH_VCavity_VNDF(float a2, float3 V, float u1, float u2, float u3)
{
    float3 H = SampleH_GGX(a2, u1, u2);
    float3 Hp = float3(-H.x, -H.y, H.z);

    float3 L = normalize(reflect(-V, H));

    float cHdL = saturate(dot(H, L));
    float cHpdL = saturate(dot(Hp, L));

    if (u3 > cHdL / (cHdL + cHpdL))
        return Hp;
    else
        return H;
}

float SampleInvC2Neg_GGX(float r1, float theta)
{
    // ???
    float tanT = tan(theta);
    float a = 1.0f / tanT;
    float G1 = 2.0f / (1 + sqrt(1.0f + 1.0f / (a * a)));

    float A = 2 * r1 / G1;
    float B = tanT;
    float A2 = A * A;
    float B2 = B * B;
    float rA2m1 = 1.0f / (A2 - 1.0f);
    float pm = sqrt(B2 * rA2m1 * rA2m1 - (A2 - B2) * rA2m1);

    float slopeX2 = B * rA2m1 + pm;
    if (A < 0 || slopeX2 > 1.0f / tanT)
        return B * rA2m1 - pm;
    return slopeX2;
}

float SampleInvC2if2_GGX(float r2, float slopeX)
{
    float s = 1;
    if (r2 <= 0.5f) // ?
        r2 = 2 * (r2 - 0.5f);
    else
    {
        s = -1;
        r2 = 2 * (0.5f - r2);
    }
    float z = s * (0.46341 * r2 - 0.73369 * r2 * r2 + 0.27385 * r2 * r2 * r2);
    z /= (0.597999 - r2 + 0.30942 * r2 * r2 + 0.093073 * r2 * r2 * r2);
    return z * sqrt(1 + slopeX * slopeX);
}

float2 SampleP22wo_11_GGX(float theta, float r1, float r2)
{
    // Special case (normal incidence)
    if (theta < 0.0001)
    {
        float r = sqrt(r1 / (1 - r1));
        float phi = 6.283185 * r;
        return float2(r * cos(phi), r * sin(phi));
    }

    float2 slope;
    slope.x = SampleInvC2Neg_GGX(r1, theta);
    slope.y = SampleInvC2if2_GGX(r2, slope.x);
    return slope;
}

// Untested
float3 SampleH_Smith_VNDF(float alphaX, float alphaY, float3 V, float u1, float u2)
{
    // Stretch V
    V.x *= alphaX;
    V.y *= alphaY;
    V = normalize(V);

    float theta = 0.0f;
    float phi = 0.0f;
    if (V.z < 0.99999f)
    {
        theta = acos(V.z);
        phi = atan2(V.y, V.x);
    }

    // Sample P22_wo(x_slope, y_slope, 1, 1)
    float2 slope = SampleP22wo_11_GGX(theta, u1, u2); // TODO: Generalize

    // Rotate
    float cosPhi = cos(phi);
    float sinPhi = sin(phi);
    slope = float2(cosPhi * slope.x - sinPhi * slope.y,
                    sinPhi * slope.x + cosPhi * slope.y);

    // Unstretch V
    slope *= float2(alphaX, alphaY);

    // Compute microfacet normal
    float invH = 1.0f / sqrt(slope.x * slope.x + slope.y * slope.y + 1.0f);
    return float3(-slope.x * invH, -slope.y * invH, invH);
}

float3 SampleH_Beckmann(float a2, float u1, float u2)
{
    float phi = 2.0 * PI * u2;
    float tan2Theta = -a2 * log(1.0 - u1);
    float cosTheta = 1.0 / sqrt(1.0 + tan2Theta);
    float sinTheta = sqrt(max(0.0, 1.0 - cosTheta * cosTheta));

    float3 H_s = normalize(float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta));
    return H_s;
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

float D_GGXAniso(float3 H, float alphaX, float alphaY)
{
    float k = H.x * H.x / (alphaX * alphaX) + H.y * H.y / (alphaY * alphaY) + H.z * H.z;
    return 1.0f / (PI * alphaX * alphaY * k * k);
}

// TODO: This gives a different D_GGXAniso
// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_anisotropy/README.md

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

float G1_GGX(float NdX, float a2)
{
    float denom = NdX + sqrt(max(0.0f, a2 + (1-a2) * NdX * NdX));
    return saturate(2 * NdX / max(0.001f, denom));
}

float G_SmithGGX(float NdL, float NdV, float a2)
{
    return G1_GGX(NdL, a2) * G1_GGX(NdV, a2);
}

float G_SmithGGXAniso(float3 L, float3 V, float alphaX, float alphaY, float3 T, float3 B, float3 N)
{
    float3 L_a = ToDefinedSpace(L, T, B, N);
    float3 V_a = ToDefinedSpace(V, T, B, N);
    float alphaL = AnisoAlphaToMaskingAlpha(L_a, alphaX, alphaY);
    float alphaV = AnisoAlphaToMaskingAlpha(V_a, alphaX, alphaY);
    return G1_GGX(L_a.z, alphaL) * G1_GGX(V_a.z, alphaV);
}

float Lambda_GGXAniso(float3 W, float alphaX, float alphaY)
{
    float aX2 = alphaX * alphaX;
    float aY2 = alphaY * alphaY;
    float k = (aX2 * W.x * W.x + aY2 * W.y * W.y) / max(0.0001f, W.z * W.z);
    return 0.5f * (-1 + sqrt(1 + k));
}

float G1_GGXAniso(float3 W, float alphaX, float alphaY)
{
    return 1.0f / max(0.001f, 1 + Lambda_GGXAniso(W, alphaX, alphaY));
}

float G_GGXAniso(float3 V, float3 L, float alphaX, float alphaY)
{
    return 1.0f / max(0.001f, 1 + Lambda_GGXAniso(V, alphaX, alphaY) + Lambda_GGXAniso(L, alphaX, alphaY));
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

// De-generalized from BSDF to BRDF
float G1_VCavity(float3 W, float3 H)
{
    float WdH = dot(W, H);
    return min(1, 2.0f * H.z * W.z / max(0.001f, WdH));
}

// Avoid Keleman simplified model, breaks VNDF
float G_VCavity(float3 V, float3 L, float3 H)
{
    return min(G1_VCavity(V, H), G1_VCavity(L, H));
}

// ================================
//  Probability Density Functions
// ================================

float Pdf_GGX(float D, float NdH, float VdH)
{
    return D * NdH / (4.0f * max(0.001f, VdH));
}

float Pdf_GGXAniso(float D, float NdH, float VdH)
{
    return D * NdH / (4.0f * max(0.001f, VdH));
}

// Nonsense, find actual PDF
float Pdf_GGX_VNDF(float Dv, float VdH)
{
    return Dv / (4.0f * max(0.001f, VdH));
}

// Needs dividing by (4.0f * max(0.001f, VdH)) after? Is this opertion applied to all PDFs assuming Torrence-Sparrow?
float Pdf_General(float D, float G1v, float3 V, float3 H, bool vndf)
{
    if (!vndf)
        return D * abs(SSpaceCosTheta(H));

    return D * G1v * abs(dot(V, H)) / abs(SSpaceCosTheta(V));
}

#endif