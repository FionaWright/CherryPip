#ifndef H_SELLMEIER_H
#define H_SELLMEIER_H

#include "Complex.hlsli"

// Curve fitting for material refractive index spectra

// Eta Tables:
// https://github.com/polyanskiy/refractiveindex.info-database/tree/main/database
//     Data Columns: λ, n(λ), k(λ)
//     Wavelength is in micrometers, multiply by 1000 to get nm
//     Not all materials contain data in the visible light spectrum
// https://refractiveindex.info/
// https://en.wikipedia.org/wiki/Sellmeier_equation

// N = Constant + B_0 λ^2 / (λ^2 - C_0) + ...
struct SellmeierEquation
{
    float Constant;
    float B[5];
    float C[5];
    bool MulLambda[5];

    float SampleN(float lambda_nano)
    {
        float lambda_micro = lambda_nano / 1000.0f;
        float lambda2 = lambda_micro * lambda_micro;

        float n2 = Constant;
        [unroll]
        for (int i = 0; i < 5; i++)
        {
            float term = B[i] / (lambda2 - C[i]);
            if (MulLambda[i])
                term *= lambda2;
            n2 += term;
        }
        return sqrt(n2);
    }
};

static const SellmeierEquation cSellmeier_TiO2 = {
    5.913f,
    { 0.2441f, 0, 0, 0, 0 },
    { 0.0803f, 0, 0, 0, 0 },
    { false, false, false, false, false }
};

static const SellmeierEquation cSellmeier_FusedSilica = {
    1.0f,
    { 0.6961663, 0.4079426, 0.8974794, 0, 0 },
    { 4.679148e-3, 0.01351206, 97.93400254, 0, 0 },
    { true, true, true, false, false }
};

static const SellmeierEquation cSellmeier_BGG = {
    1.0f,
    { 0.82725, 1.14635, 1.53923, 0, 0 },
    { 0.02978, 0.005117, 272.657, 0, 0 },
    { true, true, true, false, false }
};

static const SellmeierEquation cSellmeier_PVP = {
    1.5151f,
    { 0.00279, 5.0756e-4, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { false, false, false, false, false }
};

struct ConductorIOR
{
    float Wavelength;
    float N;
    float K;
};

#define COMPLEX_AL_SIZE 23
static const ConductorIOR cComplex_Al[COMPLEX_AL_SIZE] = {
    { 300.9, 1.34, 0.964 },
    { 310.7, 1.13, 0.616 },
    { 320.4, 0.81, 0.392 },
    { 331.5, 0.17, 0.829 },
    { 342.5, 0.14, 1.142 },
    { 354.2, 0.10, 1.419 },
    { 367.9, 0.07, 1.657 },
    { 381.5, 0.05, 1.864 },
    { 397.4, 0.05, 2.070 },
    { 413.3, 0.05, 2.275 },
    { 430.5, 0.04, 2.462 },
    { 450.9, 0.04, 2.657 },
    { 471.4, 0.05, 2.869 },
    { 495.9, 0.05, 3.093 },
    { 520.9, 0.05, 3.324 },
    { 548.6, 0.06, 3.586 },
    { 582.1, 0.05, 3.858 },
    { 616.8, 0.06, 4.152 },
    { 659.5, 0.05, 4.483 },
    { 704.5, 0.04, 4.838 },
    { 756.0, 0.03, 5.242 },
    { 821.1, 0.04, 5.727 },
    { 892.0, 0.04, 6.312 },
};

Complex SampleIor_Glass(float lambda)
{
    float n = cSellmeier_BGG.SampleN(lambda);
    return CreateComplex(n, 0);
}

Complex SampleIor_Dielectric(float lambda)
{
    float n = cSellmeier_PVP.SampleN(lambda);
    return CreateComplex(n, 0);
}

Complex SampleIor_Conductor(float lambda)
{
    ConductorIOR data[COMPLEX_AL_SIZE] = cComplex_Al;
    int size = COMPLEX_AL_SIZE;

    int i = 1;
    while (lambda < data[i].Wavelength && i < size)
        i++;

    float minLambda = data[i-1].Wavelength;
    float maxLambda = data[i].Wavelength;
    float t = lambda - minLambda / max(1e-6, maxLambda - minLambda);

    float n = lerp(data[i-1].N, data[i].N, t);
    float k = lerp(data[i-1].K, data[i].K, t);
    return CreateComplex(n, -k);
}

#endif