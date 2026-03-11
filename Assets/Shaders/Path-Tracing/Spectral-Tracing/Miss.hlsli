#ifndef H_MISS_SPECTRAL_H
#define H_MISS_SPECTRAL_H

#include "DualIncludes/HlslMath.h"

SpectralValue Miss(float3 origin, float3 direction, uint bounceIdx, float lambda)
{
    if (cFurnaceTestHHE)
        return BlackSpectralValue();
    if (cFurnaceTestHDR)
        return WhiteSpectralValue();

    SpectralValue Li = BlackSpectralValue();

    if (cEnvMapEnabled)
    {
        float2 uv;
        if (cEnvMapIsEqualArea)
            uv = EaSphereToSquare(direction);
        else
            uv = PanoSphereToSquare(direction);

        float3 envMapSampleRGB = saturate(gEnvMap.Sample(gSampler, uv).rgb);
#ifdef SINGLE_LAMBDA_RENDERING
        Li += RgbToSpectrumSample(envMapSampleRGB, eIlluminant, lambda);
#else
        Li.InitFromRGB(envMapSampleRGB, eIlluminant);
#endif
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

#ifdef SINGLE_LAMBDA_RENDERING
        Li += RgbToSpectrumSample(dirLightRGB, eIlluminant, lambda);
#else
        Spectrum spectrumDirLight;
        spectrumDirLight.InitFromRGB(dirLightRGB, eIlluminant);
        Li.Add(spectrumDirLight);
#endif
    }

    return Li;
}

#endif