#ifndef H_SPECTRUM_H
#define H_SPECTRUM_H

#include "HlslGlue.h"

// Higher is better, consider going up to 64/128
#define NUM_SPECTRUM_SAMPLES 32

#define VISIBLE_LIGHT_SPECTRUM_MIN 380
#define VISIBLE_LIGHT_SPECTRUM_MAX 780
#define VISIBLE_LIGHT_SPECTRUM_SIZE VISIBLE_LIGHT_SPECTRUM_MAX-VISIBLE_LIGHT_SPECTRUM_MIN

#define SPECTRUM_DELTA_LAMBDA VISIBLE_LIGHT_SPECTRUM_SIZE / float(NUM_SPECTRUM_SAMPLES-1)

enum SpectrumType
{
    eReflectance,
    eIlluminant
};

struct Spectrum
{
    float Samples[NUM_SPECTRUM_SAMPLES]; // Energy indexed by Wavelength (380nm-780nm)

    Spectrum(float3 rgb, SpectrumType type);

    void Add(Spectrum spectrum);
    void Sub(Spectrum spectrum);
    void Mul(Spectrum spectrum);
    void Mul(float scalar);
    void Div(Spectrum spectrum);
    void Div(float scalar);

    void Sqrt();
    void Clamp(float lo, float hi);

    void FmaCurve(float curve[32], float mult);
    void ReflectanceRgbToSpectrum(float3 rgb);
    void IlluminantRgbToSpectrum(float3 rgb);
};

Spectrum::Spectrum(float3 rgb, SpectrumType type)
{
    if (type == eReflectance)
        ReflectanceRgbToSpectrum(rgb);
    else if (type == eIlluminant)
        IlluminantRgbToSpectrum(rgb);
}

void Spectrum::Add(Spectrum spectrum)
{
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] += spectrum.Samples[i];
}

void Spectrum::Sub(Spectrum spectrum)
{
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] -= spectrum.Samples[i];
}

void Spectrum::Mul(Spectrum spectrum) // TODO: ?
{
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] *= spectrum.Samples[i];
}

void Spectrum::Mul(float scalar)
{
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] *= scalar;
}

void Spectrum::Div(Spectrum spectrum) // TODO: ?
{
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] /= spectrum.Samples[i];
}

void Spectrum::Div(float scalar)
{
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] /= scalar;
}

void Spectrum::Sqrt()
{
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] = glueSqrt(Samples[i]);
}

void Spectrum::Clamp(float lo, float hi)
{
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        Samples[i] = glueClamp(Samples[i], lo, hi);
}

#endif