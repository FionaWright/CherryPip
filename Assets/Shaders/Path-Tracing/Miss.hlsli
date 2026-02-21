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

        Li += saturate(gEnvMap.Sample(gSampler, uv).rgb);
    }

    if (cDirLightEnabled && bounceIdx >= 1)
    {
        float sunCos = dot(direction, -normalize(cbvPathTracing.DirLight));
        if (cDirLightIsDistant)
            Li += cbvPathTracing.DirLightColor * cbvPathTracing.DirLightIntensity * saturate(sunCos);
        else if (sunCos > cbvPathTracing.DirLightCosAngularRadius)
            Li += cbvPathTracing.DirLightColor * cbvPathTracing.DirLightIntensity * 100.0f;
    }

    return Li;
}

#endif