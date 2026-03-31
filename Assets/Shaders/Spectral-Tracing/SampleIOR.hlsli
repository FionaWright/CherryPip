#ifndef H_SAMPLE_IOR_H
#define H_SAMPLE_IOR_H

#include "Path-Tracing/Fresnel.hlsli"
#include "Spectral-Tracing/SpectralData/IOR.hlsli"

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
    while (lambda <= data[i].Wavelength && i < size)
        i++;

    float minLambda = data[i-1].Wavelength;
    float maxLambda = data[i].Wavelength;
    float t = (lambda - minLambda) / max(1e-6, maxLambda - minLambda);

    float n = lerp(data[i-1].N, data[i].N, t);
    float k = lerp(data[i-1].K, data[i].K, t);
    return CreateComplex(n, -k);
}

Complex SampleIor_Lambda(float lambda, bool isGlass, bool isConductor)
{
    if (isGlass)
		return SampleIor_Glass(lambda);
	else if (isConductor)
		return SampleIor_Conductor(lambda);
	else
		return SampleIor_Dielectric(lambda);
}

Complex SampleIor(SpectralContext ctx, bool isGlass, bool isConductor)
{
#if defined(SPECTRAL_SINGLE_WAVELENGTH_SAMPLING)
    float lambda = ctx.Lambda;
#elif defined(SPECTRAL_HERO_SAMPLING)
    float lambda = ctx.GetHeroLambda();
#else
	float lambda = -1;
#endif

    return SampleIor_Lambda(lambda, isGlass, isConductor);
}

#endif