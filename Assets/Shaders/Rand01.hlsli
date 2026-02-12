#ifndef H_RAND01_H
#define H_RAND01_H

#define UINT_MAX 4294967296.0
#define PI 3.141592653589793
#define ONE_MINUS_EPSILON 0x1.fffffep-1

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
float PcgRand01(inout uint state)
{
    state += 0x6D2B79F5u;
    uint z = (state ^ (state >> 15)) * (1u | state);
    z ^= z + (z ^ (z >> 7)) * (61u | z);
    return float((z ^ (z >> 14))) / UINT_MAX;
}

float3 RandDirectionCube(inout uint state)
{
    float x = PcgRand01(state) * 2. - 1.;
    float y = PcgRand01(state) * 2. - 1.;
    float z = PcgRand01(state) * 2. - 1.;
    return normalize(float3(x, y, z));
}

float3 RandDirectionUniform(inout uint state)
{
    float u = PcgRand01(state); // [0,1)
    float v = PcgRand01(state); // [0,1)

    float z = 1.0 - 2.0 * u;
    float r = sqrt(saturate(1.0 - z * z));
    float phi = 2.0 * PI * v;

    return float3(r * cos(phi), r * sin(phi), z);
}

float PcgRandGauss01(inout uint state)
{
    float theta = 2.0 * PI * PcgRand01(state);
    float rho = sqrt(-2.0 * log(PcgRand01(state)));
    return rho * cos(theta);
}

float3 RandDirectionSphere(inout uint state)
{
    float x = PcgRandGauss01(state);
    float y = PcgRandGauss01(state);
    float z = PcgRandGauss01(state);
    return normalize(float3(x, y, z));
}

float3 RandHemisphereUniformSSpace(float u1, float u2)
{
    float z = u1;                 // cos(theta)
    float r = sqrt(max(0, 1 - z*z));
    float phi = 2 * PI * u2;

    return normalize(float3(
        r * cos(phi),
        r * sin(phi),
        z
    ));
}

float3 RandHemisphereUniformWorld(float u1, float u2, float3 T, float3 B, float3 N)
{
    float3 L_s = RandHemisphereUniformSSpace(u1, u2);
    return normalize(L_s.x * T + L_s.y * B + L_s.z * N);
}

float3 RandHemisphereCosineSSpace(float u1, float u2)
{
    float r = sqrt(u1);
    float theta = 2.0f * PI * u2;

    float x = r * cos(theta);
    float y = r * sin(theta);
    float z = sqrt(1.0f - u1); // ensures cosine weighting

    return normalize(float3(x, y, z));
}

float3 RandHemisphereCosineWorld(float u1, float u2, float3 T, float3 B, float3 N)
{
    float3 L_s = RandHemisphereCosineSSpace(u1, u2);
    return normalize(L_s.x * T + L_s.y * B + L_s.z * N);
}

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

#if defined(SAMPLING_HALTON_OWEN) || defined(SAMPLING_HALTON)

// https://pbr-book.org/4ed/Sampling_and_Reconstruction/Halton_Sampler
// baseIdx is the dimension. Keep it different for each usage per ray sample, but the same across ray samples
float OwenScrambledRadicalInverse(int baseIdx, uint64_t a, uint hash)
{
    int base = c_primes.Primes[baseIdx]; // TODO: Not a fan of this file knowing about CBVs but...
    float invBase = 1.0f / (float)base;
    float invBaseM = 1;

    uint64_t reversedDigits = 0;
    int digitIndex = 0;
    while (1.0f - invBaseM < 1.0f)
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
    return min(invBaseM * (float)reversedDigits, ONE_MINUS_EPSILON);
}

float RadicalInverse(int baseIdx, uint64_t a)
{
    uint base = c_primes.Primes[baseIdx];
    float invBase = 1.0f / (float)base;
    float invBaseM = 1;

    uint64_t r = 0;
    while (a)
	{
	    uint64_t next = a / base;
        uint64_t digit = a - next * base;
        r = r * base + digit;
        invBaseM *= invBase;
        a = next;
    }
    return min((float)r * invBaseM, ONE_MINUS_EPSILON);
}

#endif

// OSRI Dimensionality:
// 0 - Subpixel Jitter X
// 1 - Subpixel Jitter Y
// 2 - Lens U (DoF)
// 3 - Lens V (DoF)
// Per Bounce (d = 4 + 6 * BounceIdx):
// d+0 - Alpha Cut Probability
// d+1 - Specular Probability (Or Reflect Probability for glass)
// d+2 - BSDF Sample U1
// d+3 - BSDF Sample U2
// d+4 - BSDF Sample U3
// d+5 - Russian Roulette Probability

uint GetHash(uint2 uvID, uint sampleIdx, uint frameNum)
{
#if defined(SAMPLING_HALTON_OWEN)
    return PrngSeed(uvID, 0, frameNum);
#elif defined(SAMPLING_HALTON) || defined(SAMPLING_INDEPENDENT)
	return PrngSeed(uvID, sampleIdx, frameNum);
#else
    return 0;
#endif
}

#define DIM_JITTER_X 0
#define DIM_JITTER_Y 1
#define DIM_LENS_U 2
#define DIM_LENS_V 3
#define BASE_DIM_COUNT 4

#define DIM_D_ALPHA 0
#define DIM_D_SPECULAR_PROB 1
#define DIM_D_BSDF_U1 2
#define DIM_D_BSDF_U2 3
#define DIM_D_BSDF_U3 4
#define DIM_D_RUSSIAN 5
#define BOUNCE_DIM_COUNT 6

uint GetBaseDim(uint bounceIdx)
{
    return BASE_DIM_COUNT + BOUNCE_DIM_COUNT * bounceIdx;
}

float Rand01(int dimension, uint64_t sampleIdx, inout uint hash)
{
#if defined(SAMPLING_HALTON_OWEN)
    return OwenScrambledRadicalInverse(dimension, sampleIdx, hash);
#elif defined(SAMPLING_HALTON)
	float val = RadicalInverse(dimension, sampleIdx);
    return frac(val + PcgRand01(hash));
#elif defined(SAMPLING_INDEPENDENT)
    return PcgRand01(hash);
#else
    return 0;
#endif
}

#endif