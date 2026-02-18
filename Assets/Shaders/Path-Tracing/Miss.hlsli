#ifndef H_MISSPS_H
#define H_MISSPS_H

#include "DualIncludes/HlslMath.h"

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
#    ifdef DIR_LIGHT_DISTANT
        Li += c_pathTracing.DirLightColor * c_pathTracing.DirLightIntensity * saturate(sunCos);
#    else
        if (sunCos > c_pathTracing.DirLightCosAngularRadius)
            Li += c_pathTracing.DirLightColor * c_pathTracing.DirLightIntensity * 100.0f;
#    endif
    }
#endif

    return Li;
}

#endif