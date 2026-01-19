#ifndef H_MATH_HLSL_H
#define H_MATH_HLSL_H

#include "HlslGlue.h"

// Only for functions that are required by both C++ and HLSL
// Keep HLSL-only functions local to the shader file

#define PI 3.141592653589793

float CopySign(float mag, float sign)
{
    return sign < 0.0f ? -glueAbs(mag) : glueAbs(mag);
}

float SafeSqrt(float x) { return glueSqrt(glueMax(0.0f, x)); }

float3 EaSquareToSphere(float2 uv)
{
    // Transform to [-1, 1]^2
    const float ax = 2.0f * uv.x - 1.0f;
    const float ay = 2.0f * uv.y - 1.0f;
    const float absax = glueAbs(ax);
    const float absay = glueAbs(ay);

    // Compute radius and angle
    const float signedDist = 1 - (absax + absay); // Signed distance to the u + v = 1 diagonal diamond
    const float d = glueAbs(signedDist);
    const float r = 1 - d;
    const float phi = (r == 0 ? 1 : (absay - absax) / r + 1) * PI / 4;

    // Compute vector
    const float y = CopySign(1 - r * r, signedDist);
    const float cosPhi = CopySign(glueCos(phi), ax);
    const float sinPhi = CopySign(glueSin(phi), ay);
    return float3(cosPhi * r * SafeSqrt(2 - r * r), y, sinPhi * r * SafeSqrt(2 - r * r));
}

float2 PanoSphereToSquare(float3 d)
{
    float lambda = atan2(d.z, d.x);    // [-pi,pi]
    float phi = glueAsin(glueClamp(d.y, -1.0f, 1.0f));
    float u = (lambda + PI) / (2.0f * PI);
    float v = (phi + 0.5 * PI) / PI;
    u = glueFrac(u);
    return float2(u, v);
}

#endif