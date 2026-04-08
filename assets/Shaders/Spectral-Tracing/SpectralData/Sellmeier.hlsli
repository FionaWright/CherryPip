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

#endif