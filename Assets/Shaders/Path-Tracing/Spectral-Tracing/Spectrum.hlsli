#ifndef H_SPECTRUM_H
#define H_SPECTRUM_H

// Higher is better, consider going up to 64/128
#define NUM_SPECTRUM_SAMPLES 32

#define VISIBLE_LIGHT_SPECTRUM_MIN 380
#define VISIBLE_LIGHT_SPECTRUM_MAX 780
#define VISIBLE_LIGHT_SPECTRUM_SIZE (VISIBLE_LIGHT_SPECTRUM_MAX - VISIBLE_LIGHT_SPECTRUM_MIN)
#define SPECTRUM_DELTA_LAMBDA (VISIBLE_LIGHT_SPECTRUM_SIZE / float(NUM_SPECTRUM_SAMPLES-1)) // 12.9

enum SpectrumType
{
    eReflectance,
    eIlluminant
};

struct Spectrum
{
    float Samples[NUM_SPECTRUM_SAMPLES]; // Energy indexed by Wavelength (380nm-780nm)

    void InitFromRGB(float3 rgb, SpectrumType type);

    void Add(Spectrum spectrum);
    void Sub(Spectrum spectrum);
    void Mul(Spectrum spectrum);
    void Mul(float scalar);
    void Div(Spectrum spectrum);
    void Div(float scalar);

    void Sqrt();
    void Clamp(float lo, float hi);
    void Normalize();

    void FmaCurve(float curve[32], float mult);
    void ReflectanceRgbToSpectrum(float3 rgb);
    void IlluminantRgbToSpectrum(float3 rgb);
};

void Spectrum::InitFromRGB(float3 rgb, SpectrumType type)
{
    // Initialize samples to 0
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] = 0.0f;

    if (type == eReflectance)
        ReflectanceRgbToSpectrum(rgb);
    else if (type == eIlluminant)
        IlluminantRgbToSpectrum(rgb);
}

void Spectrum::Add(Spectrum spectrum)
{
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] += spectrum.Samples[i];
}

Spectrum Add(Spectrum spectrumA, Spectrum spectrumB)
{
    Spectrum newSpectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        newSpectrum.Samples[i] = spectrumA.Samples[i] + spectrumB.Samples[i];
    return newSpectrum;
}

void Spectrum::Sub(Spectrum spectrum)
{
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] -= spectrum.Samples[i];
}

Spectrum Sub(Spectrum spectrumA, Spectrum spectrumB)
{
    Spectrum newSpectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        newSpectrum.Samples[i] = spectrumA.Samples[i] - spectrumB.Samples[i];
    return newSpectrum;
}

void Spectrum::Mul(Spectrum spectrum) // TODO: ?
{
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] *= spectrum.Samples[i];
}

Spectrum Mul(Spectrum spectrumA, Spectrum spectrumB)
{
    Spectrum newSpectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        newSpectrum.Samples[i] = spectrumA.Samples[i] * spectrumB.Samples[i];
    return newSpectrum;
}

void Spectrum::Mul(float scalar)
{
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] *= scalar;
}

Spectrum Mul(Spectrum spectrumA, float scalar)
{
    Spectrum newSpectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        newSpectrum.Samples[i] = spectrumA.Samples[i] + scalar;
    return newSpectrum;
}

void Spectrum::Div(Spectrum spectrum) // TODO: ?
{
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] /= spectrum.Samples[i];
}

Spectrum Div(Spectrum spectrumA, Spectrum spectrumB)
{
    Spectrum newSpectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        newSpectrum.Samples[i] = spectrumA.Samples[i] / spectrumB.Samples[i];
    return newSpectrum;
}

void Spectrum::Div(float scalar)
{
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] /= scalar;
}

Spectrum Div(Spectrum spectrumA, float scalar)
{
    Spectrum newSpectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        newSpectrum.Samples[i] = spectrumA.Samples[i] / scalar;
    return newSpectrum;
}

void Spectrum::Sqrt()
{
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] = sqrt(Samples[i]);
}

Spectrum Sqrt(Spectrum spectrum)
{
    Spectrum newSpectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        newSpectrum.Samples[i] = sqrt(spectrum.Samples[i]);
    return newSpectrum;
}

void Spectrum::Clamp(float lo, float hi)
{
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] = clamp(Samples[i], lo, hi);
}

Spectrum Clamp(Spectrum spectrum, float lo, float hi)
{
    Spectrum newSpectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        newSpectrum.Samples[i] = clamp(spectrum.Samples[i], lo, hi);
    return newSpectrum;
}

void Spectrum::Normalize()
{
    float maxVal = 0.0f;

    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        maxVal = max(maxVal, Samples[i]);

    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] /= maxVal;
}

Spectrum Normalize(Spectrum spectrum)
{
    Spectrum newSpectrum;
    float maxVal = 0.0f;

    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        maxVal = max(maxVal, spectrum.Samples[i]);

    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        newSpectrum.Samples[i] = spectrum.Samples[i] / maxVal;
    return newSpectrum;
}

#endif