#ifndef H_SPECTRUM_TO_XYZ_H
#define H_SPECTRUM_TO_XYZ_H

#include "Spectrum.h"
#include "Spectra.h"

float SampleCIE(float lambda, float curve[471])
{
    float fIdx = lambda - CIE_LAMBDA_MIN;
    int i0 = (int)floorf(fIdx);
    int i1 = min(i0+1, 470);
    float t = fIdx - i0;

    float sample0 = curve[i0];
    float sample1 = curve[i1];
    return lerp(sample0, sample1, t);
}

static const float cCIE_Y_integral = 106.856895f;

// Reimann Sum Approximation
float3 SpectrumToXYZ(Spectrum spectrum)
{
    float3 xyz = 0.0f;
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
    {
        float lambda = (float)VISIBLE_LIGHT_SPECTRUM_MIN + (float)i * (float)SPECTRUM_DELTA_LAMBDA;

        float X_lambda = SampleCIE(lambda, cCIE_X);
        float Y_lambda = SampleCIE(lambda, cCIE_Y);
        float Z_lambda = SampleCIE(lambda, cCIE_Z);
        float3 cie = float3(X_lambda, Y_lambda, Z_lambda);

        float sample = spectrum.Samples[i];
        xyz += sample * cie * (float)SPECTRUM_DELTA_LAMBDA;
    }
    return xyz / cCIE_Y_integral;
}

#endif