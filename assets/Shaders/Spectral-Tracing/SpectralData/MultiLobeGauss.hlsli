#ifndef H_MULTI_LOBE_GAUSS_H
#define H_MULTI_LOBE_GAUSS_H

#define MAX_NUM_GAUSSIANS 7

struct Gaussian
{
    float Amplitude;
    float Mean;
    float StdDev;

    float Sample(float lambda);
};

float Gaussian::Sample(float lambda)
{
    float x = (lambda - Mean) / StdDev;
    return Amplitude * exp(-0.5f * x * x);
}

struct MultiLobeGauss
{
    Gaussian Gaussians[MAX_NUM_GAUSSIANS];

    float Sample(float lambda);
};

float MultiLobeGauss::Sample(float lambda)
{
    float x = 0.0f;
    [unroll]
    for (int i = 0; i < MAX_NUM_GAUSSIANS; i++)
        x += Gaussians[i].Sample(lambda);
    return x;
}

#endif