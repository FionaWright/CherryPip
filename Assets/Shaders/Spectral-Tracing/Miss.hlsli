#ifndef H_MISS_SPECTRAL_H
#define H_MISS_SPECTRAL_H

#include "DualIncludes/HlslMath.h"

SpectralValue Miss(float3 origin, float3 direction, uint bounceIdx, SpectralContext ctx)
{
    if (cFurnaceTestHHE)
        return CreateBlackSpectralValue();
    if (cFurnaceTestHDR)
        return CreateWhiteSpectralValue();

    SpectralValue Li = CreateBlackSpectralValue();

    if (cEnvMapEnabled)
    {
        float2 uv;
        if (cEnvMapIsEqualArea)
            uv = EaSphereToSquare(direction);
        else
            uv = PanoSphereToSquare(direction);

        float3 envMapSampleRGB = saturate(gEnvMap.Sample(gSampler, uv).rgb);
        Li.FromRGB(envMapSampleRGB, eIlluminant, ctx);
    }

    if (cDirLightEnabled && bounceIdx >= 1)
    {
        float sunCos = dot(direction, -normalize(cbvPathTracing.DirLight));
        float3 dirLightRGB = float3(0, 0, 0);
        if (cDirLightIsDistant)
            dirLightRGB = cbvPathTracing.DirLightColor * cbvPathTracing.DirLightIntensity * saturate(sunCos);
        else if (sunCos > cbvPathTracing.DirLightCosAngularRadius)
            dirLightRGB = cbvPathTracing.DirLightColor * cbvPathTracing.DirLightIntensity * 100.0f;
        else
            return Li;

        Li.AddRGB(dirLightRGB, eIlluminant, lambda);
    }

    return Li;
}

#endif