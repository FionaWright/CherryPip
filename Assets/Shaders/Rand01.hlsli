#ifndef H_RAND01_H
#define H_RAND01_H

#define UINT_MAX 4294967295.0
#define PI 3.141592653589793

// https://www.pcg-random.org/
float PcgRand01(inout uint state)
{
    state = state * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    word = (word >> 22u) ^ word;
    return word / UINT_MAX;
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
    float rho = sqrt(-2 * log(PcgRand01(state)));
    return rho * cos(theta);
}

float3 RandDirectionSphere(inout uint state)
{
    float x = PcgRandGauss01(state);
    float y = PcgRandGauss01(state);
    float z = PcgRandGauss01(state);
    return normalize(float3(x, y, z));
}

float3 RandHemisphereUniform(inout uint state, float3 normal)
{
    float3 dir = RandDirectionSphere(state);
    return dir * sign(dot(normal, dir)); // If pointing away from normal, flip it
}

float3 any_perpendicular(float3 n)
{
    return abs(n.z) < 0.999 ? normalize(cross(n, float3(0,0,1))) : normalize(cross(n, float3(0,1,0)));
}

float3 RandHemisphereCosine(inout uint state, float3 normal)
{
    float u1 = PcgRand01(state);
    float u2 = PcgRand01(state);

    float r = sqrt(u1);
    float theta = 2.0f * PI * u2;

    float x = r * cos(theta);
    float y = r * sin(theta);
    float z = sqrt(1.0f - u1); // ensures cosine weighting

    // Build an orthonormal basis around 'normal'
    float3 tangent = normalize(any_perpendicular(normal));
    float3 bitangent = cross(normal, tangent);

    // Transform local (x,y,z) to world space
    return x * tangent + y * bitangent + z * normal;
}


#endif