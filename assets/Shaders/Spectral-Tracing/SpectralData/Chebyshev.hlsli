#ifndef H_CHEBYSHEV_H
#define H_CHEBYSHEV_H

#define CHEBYSHEV_NUM_COEFFICIENTS 16

struct ChebyshevPoly
{
    float Coefficients[CHEBYSHEV_NUM_COEFFICIENTS];

    float Sample(float lambda);
};

float ChebyshevPoly::Sample(float lambda)
{
    float b_next = 0.0f;
    float b_curr = 0.0f;

    lambda = (lambda - VISIBLE_LIGHT_SPECTRUM_MIN) / float(CIE_BASIS_LAMBDA_SIZE);
    lambda = 2.0f * lambda - 1.0f;
    float lambda2 = 2.0f * lambda;

    [unroll]
    for (int i = CHEBYSHEV_NUM_COEFFICIENTS-1; i >= 1; --i)
    {
        float b_temp = b_curr;
        b_curr = lambda2 * b_curr - b_next + Coefficients[i];
        b_next = b_temp;
    }

    return b_curr - b_next + Coefficients[0];
}

struct ChebyshevPiecewise2
{
    float LambdaSplit;
    ChebyshevPoly Cheb1, Cheb2;

    float Sample(float lambda);
};

float ChebyshevPiecewise2::Sample(float lambda)
{
    return lambda < LambdaSplit ? Cheb1.Sample(lambda) : Cheb2.Sample(lambda);
}

struct ChebyshevPiecewise3
{
    float LambdaSplit1, LambdaSplit2;
    ChebyshevPoly Cheb1, Cheb2, Cheb3;

    float Sample(float lambda);
};

float ChebyshevPiecewise3::Sample(float lambda)
{
    return lambda < LambdaSplit1 ? Cheb1.Sample(lambda) :
           lambda < LambdaSplit2 ? Cheb2.Sample(lambda) :
           Cheb3.Sample(lambda);
}

#endif