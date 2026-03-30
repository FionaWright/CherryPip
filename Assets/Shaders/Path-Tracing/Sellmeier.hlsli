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

// Silver
#define COMPLEX_AG_SIZE 23
static const ConductorIOR cComplex_Ag[COMPLEX_AG_SIZE] = {
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

// Gold
// TODO: Causing issues at grazing angles for some reason?
#define COMPLEX_AU_SIZE 23
static const ConductorIOR cComplex_Au[COMPLEX_AU_SIZE] = {
    { 300.9, 1.53, 1.889 },
    { 310.7, 1.53, 1.893 },
    { 320.4, 1.54, 1.898 },
    { 331.5, 1.48, 1.883 },
    { 342.5, 1.48, 1.871 },
    { 354.2, 1.50, 1.866 },
    { 367.9, 1.48, 1.895 },
    { 381.5, 1.46, 1.933 },
    { 397.4, 1.47, 1.952 },
    { 413.3, 1.46, 1.958 },
    { 430.5, 1.45, 1.948 },
    { 450.9, 1.38, 1.914 },
    { 471.4, 1.31, 1.849 },
    { 495.9, 1.04, 1.833 },
    { 520.9, 0.62, 2.081 },
    { 548.6, 0.43, 2.455 },
    { 582.1, 0.29, 2.863 },
    { 616.8, 0.21, 3.272 },
    { 659.5, 0.14, 3.697 },
    { 704.5, 0.13, 4.103 },
    { 756.0, 0.14, 4.542 },
    { 821.1, 0.16, 5.083 },
    { 892.0, 0.17, 5.663 }
};

// Copper
#define COMPLEX_CU_SIZE 23
static const ConductorIOR cComplex_Cu[COMPLEX_CU_SIZE] = {
    { 300.9, 1.40, 1.679 },
    { 310.7, 1.38, 1.729 },
    { 320.4, 1.38, 1.783 },
    { 331.5, 1.34, 1.821 },
    { 342.5, 1.36, 1.864 },
    { 354.2, 1.37, 1.916 },
    { 367.9, 1.36, 1.975 },
    { 381.5, 1.33, 2.045 },
    { 397.4, 1.32, 2.116 },
    { 413.3, 1.28, 2.207 },
    { 430.5, 1.25, 2.305 },
    { 450.9, 1.24, 2.397 },
    { 471.4, 1.25, 2.483 },
    { 495.9, 1.22, 2.564 },
    { 520.9, 1.18, 2.608 },
    { 548.6, 1.02, 2.577 },
    { 582.1, 0.70, 2.704 },
    { 616.8, 0.30, 3.205 },
    { 659.5, 0.22, 3.747 },
    { 704.5, 0.21, 4.205 },
    { 756.0, 0.24, 4.665 },
    { 821.1, 0.26, 5.180 },
    { 892.0, 0.30, 5.768 }
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
    //ConductorIOR data[COMPLEX_AG_SIZE] = cComplex_Ag;
    //int size = COMPLEX_AG_SIZE;
    //ConductorIOR data[COMPLEX_AU_SIZE] = cComplex_Au;
    //int size = COMPLEX_AU_SIZE;
    ConductorIOR data[COMPLEX_CU_SIZE] = cComplex_Cu;
    int size = COMPLEX_CU_SIZE;

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