#ifndef H_MISSPS_H
#define H_MISSPS_H

float2 DirToEaUV(float3 dir)
{
    dir = normalize(dir);
    float theta = atan2(dir.z, dir.x);   // longitude [-pi, pi]
    float phi   = asin(dir.y);         // latitude  [-pi/2, pi/2]
    float u = (theta + PI) / (2.0f * PI);         // 0..1
    float v = (sin(phi) + 1.0f) * 0.5f;               // 0..1
    return float2(u, v);
}

float3 Miss(float3 origin, float3 direction)
{
#if defined(FURNACE_TEST_EMISSIVE)
    return float3(0, 0, 0);
#elif defined(FURNACE_TEST_CLASSIC)
    return float3(1, 1, 1);
#else
    float uv = DirToEaUV(direction);
    return gEnvMap.Sample(c_sampler, uv).rgb;
#endif
}

#endif