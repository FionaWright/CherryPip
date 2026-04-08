#ifndef H_HALTON_H
#define H_HALTON_H

#ifdef __cplusplus
#include "CBV.h"
static CbvPrimes cbvPrimes;
#endif

// https://github.com/mmp/pbrt-v4/blob/master/src/pbrt/util/math.h
uint64_t MixBits(uint64_t v)
{
    v ^= (v >> 31);
    v *= 0x7fb5d329728ea185;
    v ^= (v >> 27);
    v *= 0x81dadef4bc2dd44d;
    v ^= (v >> 33);
    return v;
}

// https://github.com/mmp/pbrt-v4/blob/master/src/pbrt/util/math.h
// Returns the ith element of a random permutation of l values based on the seed p
uint PermutationElement(uint i, uint l, uint p) {
    uint w = l - 1;
    w |= w >> 1;
    w |= w >> 2;
    w |= w >> 4;
    w |= w >> 8;
    w |= w >> 16;
    do
    {
        i ^= p;
        i *= 0xe170893d;
        i ^= p >> 16;
        i ^= (i & w) >> 4;
        i ^= p >> 8;
        i *= 0x0929eb3f;
        i ^= p >> 23;
        i ^= (i & w) >> 1;
        i *= 1 | p >> 27;
        i *= 0x6935fa69;
        i ^= (i & w) >> 11;
        i *= 0x74dcb303;
        i ^= (i & w) >> 2;
        i *= 0x9e501cc3;
        i ^= (i & w) >> 2;
        i *= 0xc860a3df;
        i &= w;
        i ^= i >> 5;
    } while (i >= l);
    return (i + p) % l;
}

int GetPrime(int idx)
{
#if defined(SAMPLING_HALTON_OWEN) || defined(SAMPLING_HALTON) || defined(SAMPLING_HALTON_APPLE) || defined(__cplusplus)
    return cbvPrimes.Primes[idx]; // TODO: Not a fan of this file knowing about CBVs but...
#else
    return 0;
#endif
}

// https://pbr-book.org/4ed/Sampling_and_Reconstruction/Halton_Sampler
// baseIdx is the dimension. Keep it different for each usage per ray sample, but the same across ray samples
float OwenScrambledRadicalInverse(int baseIdx, uint64_t a, uint hash)
{
    int base = GetPrime(baseIdx);
    float invBase = 1.0f / (float)base;
    float invBaseM = 1;

    uint64_t reversedDigits = 0;
    int digitIndex = 0;
    //while (1.0f - invBaseM < 1.0f)
    while (a > 0)
    {
        uint64_t next = a / base;
        int digitValue = (int)(a - next * base);
        uint digitHash = (uint)MixBits(hash ^ reversedDigits);
        digitValue = PermutationElement(digitValue, base, digitHash);
        reversedDigits = reversedDigits * base + digitValue;
        invBaseM *= invBase;
        ++digitIndex;
        a = next;

    }
    return glueMin(invBaseM * (float)reversedDigits, (float)ONE_MINUS_EPSILON);
}

float RadicalInverse(int baseIdx, uint64_t a)
{
    uint base = GetPrime(baseIdx);
    float invBase = 1.0f / (float)base;
    float invBaseM = 1;

    uint64_t r = 0;
    while (a > 0)
	{
	    uint64_t next = a / base;
        uint64_t digit = a - next * base;
        r = r * base + digit;
        invBaseM *= invBase;
        a = next;
    }
    return glueMin((float)r * invBaseM, (float)ONE_MINUS_EPSILON);
}

float AppleRadicalInverse(int baseIdx, uint64_t a)
{
    uint b = GetPrime(baseIdx);
    float f = 1.0f;
    float invB = 1.0f / b;

    float r = 0;
    while (a > 0)
    {
        f = f * invB;
        r = r + f * float(a % b);
        a = a / b;
    }

    return glueMin(r, (float)ONE_MINUS_EPSILON);
}

#define DIM_JITTER_X 0
#define DIM_JITTER_Y 1
#define DIM_LENS_U 2
#define DIM_LENS_V 3
#define BASE_DIM_COUNT 4

#define DIM_D_SPECULAR_PROB 0
#define DIM_D_BSDF_U1 1
#define DIM_D_BSDF_U2 2
#define DIM_D_BSDF_U3 3
#define DIM_D_ALPHA 4
#define BOUNCE_DIM_COUNT 5

uint GetBaseDim(uint bounceIdx)
{
    return BASE_DIM_COUNT + BOUNCE_DIM_COUNT * bounceIdx;
}

#endif