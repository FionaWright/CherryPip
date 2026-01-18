#ifndef H_MISSPS_H
#define H_MISSPS_H

float CopySign(float mag, float sign)
{
    return sign < 0.0f ? -abs(mag) : abs(mag);
}

float SafeSqrt(float x) { return sqrt(max(0.0f, x)); }

float2 EaSphereToSquare(float3 d)
{
    float x = abs(d.x);
    float y = abs(d.y);
    float z = abs(d.z);
    float r = SafeSqrt(1 - y);
    float a = max(x, z);
    float b = min(x, z);
    b = a == 0 ? 0 : b / a;

    float phi = atan(b) * 2.0f / PI; // Can use polynomial to optimize here?
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
    float phi = asin(clamp(d.y, -1.0f, 1.0f));
    float u = (lambda + PI) / (2.0f * PI);
    float v = (phi + 0.5 * PI) / PI;
    u = frac(u);
    return float2(u, 1-v);
}

float3 Miss(float3 origin, float3 direction, uint bounceIdx)
{
#if defined(FURNACE_TEST_HEMI_HEMI_EMIT)
    return float3(0, 0, 0);
#elif defined(FURNACE_TEST_HEMI_DIR_REFLECT)
    return float3(1, 1, 1);
#endif

    float3 Li = float3(0, 0, 0);

#ifdef ENV_MAP_ENABLED
#    if defined(ENV_MAP_EA)
    float2 uv = EaSphereToSquare(direction);
#    else
    float2 uv = PanoSphereToSquare(direction);
#    endif

    Li += saturate(gEnvMap.Sample(c_sampler, uv).rgb);
#endif

#ifdef DIR_LIGHT_ENABLED
    if (bounceIdx >= 1)
    {
        float sunCos = dot(direction, -normalize(c_pathTracing.DirLight));
        if (sunCos > c_pathTracing.DirLightCosAngularRadius)
            Li += c_pathTracing.DirLightColor * c_pathTracing.DirLightIntensity;
    }
#endif

    return Li;
}

#endif