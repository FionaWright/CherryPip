#ifndef H_SELLMEIER_H
#define H_SELLMEIER_H

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

float SampleIorN_Glass(float lambda)
{
    return cSellmeier_BGG.SampleN(lambda);
}

float SampleIorN_Dielectric(float lambda)
{
    return cSellmeier_PVP.SampleN(lambda);
}

float SampleIorN_Conductor(float lambda)
{
    return 1.5f; // TODO
}

#endif