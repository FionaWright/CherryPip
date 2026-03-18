#ifndef H_SPECTRUM_H
#define H_SPECTRUM_H

// Higher is better, consider going up to 64/128
#define NUM_SPECTRUM_SAMPLES 64

#define VISIBLE_LIGHT_SPECTRUM_MIN 390
#define VISIBLE_LIGHT_SPECTRUM_MAX 720
#define VISIBLE_LIGHT_SPECTRUM_SIZE (VISIBLE_LIGHT_SPECTRUM_MAX - VISIBLE_LIGHT_SPECTRUM_MIN)
#define SPECTRUM_DELTA_LAMBDA (VISIBLE_LIGHT_SPECTRUM_SIZE / float(NUM_SPECTRUM_SAMPLES-1)) 

enum SpectrumType
{
    eReflectance,
    eIlluminant
};

struct Spectrum
{
    float Samples[NUM_SPECTRUM_SAMPLES]; // Energy indexed by Wavelength (380nm-780nm)

    void InitFromRGB(float3 rgb, SpectrumType type);

    float Sample(float lambda);

    void Add(Spectrum spectrum);
    void Add(float scalar);
    void Sub(Spectrum spectrum);
    void Sub(float scalar);
    void Mul(Spectrum spectrum);
    void Mul(float scalar);
    void Div(Spectrum spectrum);
    void Div(float scalar);

    void Sqrt();
    void Clamp(float lo, float hi);
    void Normalize();
    void Exp();

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

float Spectrum::Sample(float lambda)
{
    float fIdx = (lambda - VISIBLE_LIGHT_SPECTRUM_MIN) / SPECTRUM_DELTA_LAMBDA;
    fIdx = clamp(fIdx, 0, NUM_SPECTRUM_SAMPLES-1);
    int i0 = (int)floor(fIdx);
    int i1 = min(i0+1, NUM_SPECTRUM_SAMPLES-1);
    float t = fIdx - i0;
    return lerp(Samples[i0], Samples[i1], t);
}

void Spectrum::Add(Spectrum spectrum)
{
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] += spectrum.Samples[i];
}

void Spectrum::Add(float scalar)
{
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] += scalar;
}

Spectrum Add(Spectrum spectrumA, Spectrum spectrumB)
{
    Spectrum newSpectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        newSpectrum.Samples[i] = spectrumA.Samples[i] + spectrumB.Samples[i];
    return newSpectrum;
}

Spectrum Add(Spectrum spectrumA, float scalar)
{
    Spectrum newSpectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        newSpectrum.Samples[i] = spectrumA.Samples[i] + scalar;
    return newSpectrum;
}

void Spectrum::Sub(Spectrum spectrum)
{
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] -= spectrum.Samples[i];
}

void Spectrum::Sub(float scalar)
{
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] -= scalar;
}

Spectrum Sub(Spectrum spectrumA, Spectrum spectrumB)
{
    Spectrum newSpectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        newSpectrum.Samples[i] = spectrumA.Samples[i] - spectrumB.Samples[i];
    return newSpectrum;
}

Spectrum Sub(Spectrum spectrumA, float scalar)
{
    Spectrum newSpectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        newSpectrum.Samples[i] = spectrumA.Samples[i] - scalar;
    return newSpectrum;
}

Spectrum Sub(float scalar, Spectrum spectrumA)
{
    Spectrum newSpectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        newSpectrum.Samples[i] = scalar - spectrumA.Samples[i];
    return newSpectrum;
}

void Spectrum::Mul(Spectrum spectrum)
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

void Spectrum::Div(Spectrum spectrum)
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

Spectrum Div(float scalar, Spectrum spectrumA)
{
    Spectrum newSpectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        newSpectrum.Samples[i] = scalar / spectrumA.Samples[i];
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

void Spectrum::Exp()
{
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] = exp(Samples[i]);
}

Spectrum Exp(Spectrum spectrum)
{
    Spectrum newSpectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        newSpectrum.Samples[i] = exp(spectrum.Samples[i]);
    return newSpectrum;
}

Spectrum Lerp(Spectrum a, Spectrum b, float t)
{
    Spectrum newSpectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        newSpectrum.Samples[i] = lerp(a.Samples[i], b.Samples[i], t);
    return newSpectrum;
}

Spectrum Lerp(Spectrum a, float end, float t)
{
    Spectrum newSpectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        newSpectrum.Samples[i] = lerp(a.Samples[i], end, t);
    return newSpectrum;
}

Spectrum Lerp(float start, Spectrum a, float t)
{
    Spectrum newSpectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        newSpectrum.Samples[i] = lerp(start, a.Samples[i], t);
    return newSpectrum;
}

#endif