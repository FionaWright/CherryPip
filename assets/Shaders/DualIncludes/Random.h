#ifndef H_RAND01_H
#define H_RAND01_H

#ifndef UINT_MAX
#define UINT_MAX 4294967296.0
#endif

#ifndef PI
#define PI 3.141592653589793
#endif

#ifndef ONE_MINUS_EPSILON
#define ONE_MINUS_EPSILON 0x1.fffffep-1
#endif

#include "HlslGlue.h"
#include "Halton.h"
#include "Path-Tracing/MacroConstants.hlsli"

uint wang_hash(uint a) {
    a = (a ^ 61u) ^ (a >> 16);
    a *= 9u;
    a = a ^ (a >> 4);
    a *= 0x27d4eb2du;
    a = a ^ (a >> 15);
    return a;
}

uint PrngSeed(uint2 pixel, uint sample_i, uint temporal_i) {
    // Random big primes
    // XOR the temporal frame number to avoid accumulated patterns
    return wang_hash(
        pixel.x * 374761393u +
        pixel.y * 668265263u +
        (sample_i * 1597334677u) ^
        (temporal_i * 3812015801u)
    );
}

// https://www.pcg-random.org/
float PcgRand01(GLUE_INOUT(uint) state)
{
    state += 0x6D2B79F5u;
    uint z = (state ^ (state >> 15)) * (1u | state);
    z ^= z + (z ^ (z >> 7)) * (61u | z);
    return float((z ^ (z >> 14))) / UINT_MAX;
}

float3 RandDirectionCube(GLUE_INOUT(uint) state)
{
    float x = PcgRand01(state) * 2. - 1.;
    float y = PcgRand01(state) * 2. - 1.;
    float z = PcgRand01(state) * 2. - 1.;
    return glueNormalize(float3(x, y, z));
}

float3 RandDirectionUniform(GLUE_INOUT(uint) state)
{
    float u = PcgRand01(state); // [0,1)
    float v = PcgRand01(state); // [0,1)

    float z = 1.0 - 2.0 * u;
    float r = glueSqrt(glueSaturate(1.0 - z * z));
    float phi = 2.0 * PI * v;

    return float3(r * glueCos(phi), r * glueSin(phi), z);
}

float PcgRandGauss01(GLUE_INOUT(uint) state)
{
    float theta = 2.0 * PI * PcgRand01(state);
    float rho = glueSqrt(-2.0 * glueLog(PcgRand01(state)));
    return rho * glueCos(theta);
}

float3 RandDirectionSphere(GLUE_INOUT(uint) state)
{
    float x = PcgRandGauss01(state);
    float y = PcgRandGauss01(state);
    float z = PcgRandGauss01(state);
    return glueNormalize(float3(x, y, z));
}

float3 RandHemisphereUniformSSpace(float u1, float u2)
{
    float z = u1;                 // glueCos(theta)
    float r = glueSqrt(glueMax(0.0f, 1.0f - z*z));
    float phi = 2 * PI * u2;

    return glueNormalize(float3(
        r * glueCos(phi),
        r * glueSin(phi),
        z
    ));
}

float3 RandHemisphereCosineSSpace(float u1, float u2)
{
    float r = glueSqrt(u1);
    float theta = 2.0f * PI * u2;

    float x = r * glueCos(theta);
    float y = r * glueSin(theta);
    float z = glueSqrt(1.0f - u1); // ensures cosine weighting

    return glueNormalize(float3(x, y, z));
}

struct RngInfo
{
    uint SampleIdx;
    uint GlobalSampleIdx;
    uint HashScramble;
    uint BounceBaseDimension;
    uint IndependentRngState; // Modified during independent sampling
};

uint GetHashScramble(uint2 pos, uint sampleIdx, uint frameIdx)
{
    if (cRngSamplingStrategy == eHalton || cRngSamplingStrategy == eHaltonApple)
        return PrngSeed(pos, 0, 0);

    if (cRngSamplingStrategy == eOwenScrambledHalton)
        return PrngSeed(pos, 0, 0);

    return 0;
}

float Rand01(int dimension, GLUE_INOUT(RngInfo) rngInfo)
{
    if (cRngSamplingStrategy == eIndependent)
        return PcgRand01(rngInfo.IndependentRngState);

    if (cRngSamplingStrategy == eHalton)
    {
        uint64_t extraHash = rngInfo.HashScramble + rngInfo.GlobalSampleIdx;
        return RadicalInverse(dimension, extraHash);
    }

    if (cRngSamplingStrategy == eHaltonApple)
    {
        uint64_t extraHash = rngInfo.HashScramble + rngInfo.GlobalSampleIdx;
        return AppleRadicalInverse(dimension, extraHash);
    }

    if (cRngSamplingStrategy == eOwenScrambledHalton)
    {
        uint64_t extraHash = rngInfo.HashScramble + rngInfo.GlobalSampleIdx;
        return OwenScrambledRadicalInverse(dimension, extraHash, rngInfo.HashScramble);
    }

    return 0;
}

float Rand01_Bounce(int dimension, GLUE_INOUT(RngInfo) rngInfo)
{
    // Non-scambled Halton breaks at high dimensionality
    if (cRngSamplingStrategy == eIndependent || cRngSamplingStrategy == eHalton || cRngSamplingStrategy == eHaltonApple)
        return PcgRand01(rngInfo.IndependentRngState);

    if (cRngSamplingStrategy == eOwenScrambledHalton)
        dimension += rngInfo.BounceBaseDimension;

    return Rand01(dimension, rngInfo);
}

#ifndef __cplusplus

#include "ShadingFrame.hlsli"

float3 RandHemisphereCosineWorld(float u1, float u2, ShadingFrame frame)
{
    float3 L_s = RandHemisphereCosineSSpace(u1, u2);
    return frame.ToWorld(L_s);
}

float3 RandHemisphereUniformWorld(float u1, float u2, ShadingFrame frame)
{
    float3 L_s = RandHemisphereUniformSSpace(u1, u2);
    return frame.ToWorld(L_s);
}

#endif

#endif