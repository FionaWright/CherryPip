#ifndef H_COLOR_SPECTRUMS_H
#define H_COLOR_SPECTRUMS_H

Spectrum BlackSpectrum();
Spectrum WhiteSpectrum_E();
Spectrum WhiteSpectrum_D65();

#include "Spectral-Tracing/SpectralUtils.hlsli"
#include "Spectral-Tracing/SpectralData/D65StandardIlluminant.hlsli"
#include "Spectral-Tracing/SpectralData/D65_MLG.hlsli"

Spectrum BlackSpectrum()
{
    Spectrum spectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        spectrum.Samples[i] = 0.0f;
    return spectrum;
}

// Doesn't have to be 1.0f, just uniform to be white
Spectrum WhiteSpectrum_E()
{
    Spectrum spectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        spectrum.Samples[i] = 1.0f;
    return spectrum;
}

float SampleD65(float lambda)
{
    float fIdx = (lambda - WHITE_D65_LAMBDA_MIN) / WHITE_D65_LAMBDA_DELTA;
    int i0 = (int)clamp(floor(fIdx), 0, WHITE_D65_COUNT-1);
    int i1 = min(i0+1, WHITE_D65_COUNT-1);
    float t = fIdx - i0;
    return lerp(cWhiteD65[i0], cWhiteD65[i1], t) * 0.01f; // Normalize
}

float SampleD65_MLG(float lambda)
{
    float normalizedLambda = (lambda - WHITE_D65_LAMBDA_MIN) / float(WHITE_D65_LAMBDA_SIZE);
    return cWhiteD65_MLG.Sample(normalizedLambda) * 0.01f;
}

Spectrum WhiteSpectrum_D65()
{
    Spectrum spectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
    {
        float lambda = IndexToLambda((float)i);
        spectrum.Samples[i] = SampleD65(lambda);
    }
    return spectrum;
}

Spectrum RedSpectrum()
{
    Spectrum spectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
    {
        float lambda = VISIBLE_LIGHT_SPECTRUM_MIN + i * SPECTRUM_DELTA_LAMBDA;
        spectrum.Samples[i] = (lambda >= 600.0f && lambda <= 700.0f) ? 1.0f : 0.0f;
    }
    return spectrum;
}

Spectrum GreenSpectrum()
{
    Spectrum spectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
    {
        float lambda = VISIBLE_LIGHT_SPECTRUM_MIN + i * SPECTRUM_DELTA_LAMBDA;
        spectrum.Samples[i] = (lambda >= 500.0f && lambda <= 570.0f) ? 1.0f : 0.0f;
    }
    return spectrum;
}

#endif