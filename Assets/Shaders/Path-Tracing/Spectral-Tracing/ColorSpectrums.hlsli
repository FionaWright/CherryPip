#ifndef H_COLOR_SPECTRUMS_H
#define H_COLOR_SPECTRUMS_H

Spectrum BlackSpectrum()
{
    Spectrum spectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        spectrum.Samples[i] = 0.0f;
    return spectrum;
}

// Doesn't have to be 1.0f, just uniform to be white
Spectrum WhiteSpectrum()
{
    Spectrum spectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        spectrum.Samples[i] = 1.0f;
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