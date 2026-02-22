#ifndef H_SPECTRUM_TO_XYZ_H
#define H_SPECTRUM_TO_XYZ_H

#include "Path-Tracing/Spectral-Tracing/Spectrum.hlsli"
#include "Path-Tracing/Spectral-Tracing/Spectra.hlsli"
#include "Path-Tracing/Spectral-Tracing/SpectralUtils.hlsli"

float SampleCIE(float lambda, float curve[471])
{
    float fIdx = (lambda - CIE_LAMBDA_MIN) / CIE_LAMBDA_DELTA;
    int i0 = (int)floor(fIdx);
    int i1 = min(i0+1, 470);
    float t = fIdx - i0;

    float sample0 = curve[i0];
    float sample1 = curve[i1];
    return lerp(sample0, sample1, t);
}

float SampleAverageCIE(float lambdaStart, float lambdaEnd, float curve[471])
{
    if (lambdaEnd <= CIE_LAMBDA_MIN)
        return curve[0];
    if (lambdaStart >= CIE_LAMBDA_MAX)
        return curve[470];

    float sum = 0.0f;

    // Values at start and end treated as constant to +/- infinity
    if (lambdaStart < CIE_LAMBDA_MIN)
        sum += curve[0] * (CIE_LAMBDA_MIN - lambdaStart);
    if (lambdaEnd > CIE_LAMBDA_MAX)
        sum += curve[470] * (lambdaEnd - CIE_LAMBDA_MAX);

    int startIdx = lambdaStart - CIE_LAMBDA_MIN;
    int endIdx = lambdaEnd - CIE_LAMBDA_MIN;
    for (int i = startIdx; i < 470 && i < endIdx; i++)
    {
        float lambdaI = i + CIE_LAMBDA_MIN;
        float segLambdaStart = max(lambdaStart, lambdaI);
        float segLambdaEnd = min(lambdaEnd, lambdaI+1);

        float lerp1 = lerp(curve[i], curve[i + 1], segLambdaStart - lambdaI);
        float lerp2 = lerp(curve[i], curve[i + 1], segLambdaEnd - lambdaI);

        sum += 0.5f * (lerp1 + lerp2) * (segLambdaEnd - segLambdaStart);
    }
    return sum / (lambdaEnd - lambdaStart);
}

static const float cCIE_Y_integral = 106.856895f; // TODO: WRONG?

// Reimann Sum Approximation
float3 SpectrumToXYZ(Spectrum spectrum)
{
    float3 xyz = 0.0f;

    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
    {
        float lambda = IndexToLambda((float)i);

        float X_lambda = SampleCIE(lambda, cCIE_X);
        float Y_lambda = SampleCIE(lambda, cCIE_Y);
        float Z_lambda = SampleCIE(lambda, cCIE_Z);
        float3 cie = float3(X_lambda, Y_lambda, Z_lambda);

        float sample = spectrum.Samples[i];
        xyz += sample * cie;
    }

    // Normalization
    xyz *= (float)SPECTRUM_DELTA_LAMBDA;
    xyz /= cCIE_Y_integral;

    return xyz;
}

float3 SpectrumToRGB(Spectrum spectrum)
{
    static const float3x3 cMatXyzToRgb =
    {
        3.240479f, -1.537150f, -0.498535f,
        -0.969256f, 1.875991f, 0.041556f,
        0.055648f, -0.204043f, 1.057311f
    };

    float3 xyz = SpectrumToXYZ(spectrum);
    float3 rgb = mul(cMatXyzToRgb, xyz);
    return rgb;
}

#endif