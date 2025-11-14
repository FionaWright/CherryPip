#ifndef H_MISSPS_H
#define H_MISSPS_H

float2 DirToEaUV(float3 d)
{
    float lambda = atan2(d.z, d.x);    // [-pi,pi]
    float phi = asin(clamp(d.y, -1.0f, 1.0f));
    float u = (lambda + PI) / (2.0f * PI);
    float v = (phi + 0.5 * PI) / PI;
    u = frac(u);
    return float2(u, 1-v);
}

float3 Miss(float3 origin, float3 direction)
{
#if defined(FURNACE_TEST_EMISSIVE)
    return float3(0, 0, 0);
#elif defined(FURNACE_TEST_CLASSIC)
    return float3(1, 1, 1);
#else
    float2 uv = DirToEaUV(direction);
    return saturate(gEnvMap.Sample(c_sampler, uv).rgb);
#endif
}

#endif