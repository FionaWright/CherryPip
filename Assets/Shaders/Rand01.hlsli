#ifndef H_RAND01_H
#define H_RAND01_H

#define UINT_MAX 4294967296.0
#define PI 3.141592653589793

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
float PcgRand01_X(inout uint state)
{
    state = state * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    word = (word >> 22u) ^ word;
    return word / UINT_MAX;
}

float PcgRand01(inout uint state)
{
    state += 0x6D2B79F5u;
    uint z = (state ^ (state >> 15)) * (1u | state);
    z ^= z + (z ^ (z >> 7)) * (61u | z);
    return float((z ^ (z >> 14))) / UINT_MAX;
}

uint4 Pcg4d(uint4 v)
{
    v = v * 1664525u + 1013904223u;
    v.x += v.y*v.w;
    v.y += v.z*v.x;
    v.z += v.x*v.y;
    v.w += v.y*v.z;
    v ^= v >> 16u;
    v.x += v.y*v.w;
    v.y += v.z*v.x;
    v.z += v.x*v.y;
    v.w += v.y*v.z;
    return v;
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

float3 RandHemisphereUniformSSpace(inout uint state)
{
    float u1 = PcgRand01(state);
    float u2 = PcgRand01(state);

    float z = u1;                 // cos(theta)
    float r = sqrt(max(0, 1 - z*z));
    float phi = 2 * PI * u2;

    return normalize(float3(
        r * cos(phi),
        r * sin(phi),
        z
    ));
}

float3 RandHemisphereUniformWorld(inout uint state, float3 T, float3 B, float3 N)
{
    float3 L_s = RandHemisphereUniformSSpace(state);
    return normalize(L_s.x * T + L_s.y * B + L_s.z * N);
}

float3 RandHemisphereCosineSSpace(inout uint state)
{
    float u1 = PcgRand01(state);
    float u2 = PcgRand01(state);

    float r = sqrt(u1);
    float theta = 2.0f * PI * u2;

    float x = r * cos(theta);
    float y = r * sin(theta);
    float z = sqrt(1.0f - u1); // ensures cosine weighting

    return normalize(float3(x, y, z));
}

float3 RandHemisphereCosineWorld(inout uint state, float3 T, float3 B, float3 N)
{
    float3 L_s = RandHemisphereCosineSSpace(state);
    return normalize(L_s.x * T + L_s.y * B + L_s.z * N);
}


#endif