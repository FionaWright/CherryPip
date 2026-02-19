#ifndef H_MISSPS_H
#define H_MISSPS_H

#include "DualIncludes/HlslMath.h"

float3 Miss(float3 origin, float3 direction, uint bounceIdx)
{
    if (cFurnaceTestHHE)
        return float3(0, 0, 0);
    if (cFurnaceTestHDR)
        return float3(1, 1, 1);

    float3 Li = float3(0, 0, 0);

    if (cEnvMapEnabled)
    {
        float2 uv;
        if (cEnvMapIsEqualArea)
            uv = EaSphereToSquare(direction);
        else
            uv = PanoSphereToSquare(direction);

        Li += saturate(gEnvMap.Sample(c_sampler, uv).rgb);
    }

    if (cDirLightEnabled && bounceIdx >= 1)
    {
        float sunCos = dot(direction, -normalize(c_pathTracing.DirLight));
        if (cDirLightIsDistant)
            Li += c_pathTracing.DirLightColor * c_pathTracing.DirLightIntensity * saturate(sunCos);
        else if (sunCos > c_pathTracing.DirLightCosAngularRadius)
            Li += c_pathTracing.DirLightColor * c_pathTracing.DirLightIntensity * 100.0f;
    }

    return Li;
}

#endif