#ifndef H_IOR_TABLE_H
#define H_IOR_TABLE_H

struct IorTableEntry
{
    float Wavelength; // nm
    float N; // Phase Velocity
    float K; // Extinction Coefficient
};

struct SellmeierEquation
{
    float Constant;
    float B[5];
    float C[5];
    bool MulLambda[5];

    float SampleN(float lambda)
    {
        float n2 = Constant;
        for (int i = 0; i < 5; i++)

    }
};

struct CauchyEquation
{
    float A;
    float B;
};

#endif