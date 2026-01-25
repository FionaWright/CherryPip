#ifndef H_MATH_HLSL_H
#define H_MATH_HLSL_H

#include "HlslGlue.h"

// Only for functions that are required by both C++ and HLSL
// Keep HLSL-only functions local to the shader file

#ifndef PI
#define PI 3.141592653589793
#endif

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

float2 EaSphereToSquare(float3 d)
{
    float x = glueAbs(d.x);
    float y = glueAbs(d.y);
    float z = glueAbs(d.z);
    float r = SafeSqrt(1 - y);
    float a = glueMax(x, z);
    float b = glueMin(x, z);
    b = a == 0 ? 0 : b / a;

    float phi = glueAtan(b) * 2.0f / PI; // Can use polynomial to optimize here?
    if (x < z)
        phi = 1 - phi;

    float v = phi * r;
    float u = r - v;
    if (d.y < 0)
    {
        float t = u;
        u = 1 - v;
        v = 1 - t;
    }
    u = CopySign(u, d.x);
    v = CopySign(v, d.z);
    return float2(0.5f * (u + 1), 0.5f * (v + 1));
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

float3 CubemapCubeToSphere(uint face, float2 uv)
{
    float3 d;
    switch (face)
    {
    case 0: // +X
        d = float3(1.0, -uv.y, -uv.x);
        break;
    case 1: // -X
        d = float3(-1.0, -uv.y, uv.x);
        break;
    case 2: // +Y
        d = float3(uv.x, 1.0, uv.y);
        break;
    case 3: // -Y
        d = float3(uv.x, -1.0, -uv.y);
        break;
    case 4: // +Z
        d = float3(uv.x, -uv.y, 1.0);
        break;
    case 5: // -Z
        d = float3(-uv.x, -uv.y, -1.0);
        break;
    default:
        return float3(0,0,0);
    }

    return glueNormalize(d);
}

float Luminance(float3 color)
{
    return glueDot(color, float3(0.2126,0.7152,0.0722));
}

#endif