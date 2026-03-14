#ifndef H_MICROFACET_H
#define H_MICROFACET_H

#include "Random.h"
#include "MathUtils.hlsli"

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

// Returns alpha', a better alpha for beckmann distributions
float WaltersTrick(float alpha, float NdL)
{
    return 1.2f - 0.2f * sqrt(abs(NdL)) * alpha;
}

float2 AlphaToAnisoAlpha(float alpha, float anisotropyStrength)
{
    //float strength = clamp(anisotropyStrength, -0.99f, 0.99f);
    //float aspect = sqrt(1.0f - 0.9 * abs(strength));
    //return float2(alpha / aspect, alpha * aspect);

    // https://blog.selfshadow.com/publications/s2017-shading-course/imageworks/s2017_pbs_imageworks_slides_v2.pdf
    return float2(alpha * (1 + anisotropyStrength), alpha * (1 - anisotropyStrength));

    // Hack?:
    //float strength2 = anisotropyStrength * anisotropyStrength;
    //return float2(lerp(alpha, 1.0f, strength2), alpha);
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

// ================================
//  Normal Distribution Functions
// ================================

// For raster backends
float D_GGX(float NdH, float roughness)
{
    float alpha = roughness * roughness;
    float a2 = alpha * alpha;

    float denominator = (NdH * NdH * (a2 - 1.0f) + 1.0f);
    return a2 / max(0.001f, PI * denominator * denominator);
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

#ifdef SPECTRAL
SpectralValue F_Schlick_Spectral(float VdH, SpectralValue F0)
{
    float k = pow(1.0f - VdH, 5);
    return Mul(Add(F0, Sub(1.0f, F0)), k);
}
#endif

// ================================
//  Geometry Masking Functions
// ================================

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

#endif