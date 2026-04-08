#ifndef H_SPECTRAL_DEBUG_UTILS_H
#define H_SPECTRAL_DEBUG_UTILS_H

float3 WavelengthToRGB(float wavelength)
{
    float r=0, g=0, b=0;

    if (wavelength >= 380 && wavelength < 440) 
    {
        r = -(wavelength - 440) / (440 - 380);
        g = 0;
        b = 1;
    }
    else if (wavelength < 490) 
    {
        r = 0;
        g = (wavelength - 440) / (490 - 440);
        b = 1;
    }
    else if (wavelength < 510) 
    {
        r = 0;
        g = 1;
        b = -(wavelength - 510) / (510 - 490);
    }
    else if (wavelength < 580) 
    {
        r = (wavelength - 510) / (580 - 510);
        g = 1;
        b = 0;
    }
    else if (wavelength < 645)
    {
        r = 1;
        g = -(wavelength - 645) / (645 - 580);
        b = 0;
    }
    else if (wavelength <= 780) 
    {
        r = 1;
        g = 0;
        b = 0;
    }

    float factor;
    if (wavelength < 420)
        factor = 0.3 + 0.7*(wavelength-380)/(420-380);
    else if (wavelength <= 700)
        factor = 1.0;
    else
        factor = 0.3 + 0.7*(780-wavelength)/(780-700);

    return float3(r,g,b) * factor;
}

[noinline] // Prevent inlining as it would otherwise explode compile time
float3 RoundTripTest(float3 lrgb)
{
    Spectrum s;

    Spectrum whiteD65 = WhiteSpectrum_D65();

    s.InitFromRGB(lrgb, eReflectance);
    s.Mul(whiteD65); // Reflectance -> Radiance

    return SpectrumToRGB(s);
}

//[noinline] // Prevent inlining as it would otherwise explode compile time
float3 RoundTripTest_v2(float3 lrgb, in SpectralContext ctx)
{
    SpectralValue s;
    s.FromRGB(lrgb, eIlluminant, ctx);
    return s.ToRGB(ctx);
}

#endif