#ifndef H_SPECTRAL_VALUE_H
#define H_SPECTRAL_VALUE_H

#ifdef SINGLE_LAMBDA_RENDERING
#    define SpectralValue float
#else
#    define SpectralValue Spectrum
#endif

#ifdef SINGLE_LAMBDA_RENDERING
#    define WhiteSpectralValue() 1.0f
#else
#    define WhiteSpectralValue() WhiteSpectrum_E()
#endif

#ifdef SINGLE_LAMBDA_RENDERING
#    define BlackSpectralValue() 0.0f
#else
#    define BlackSpectralValue() BlackSpectrum()
#endif

#ifdef SINGLE_LAMBDA_RENDERING
float Add(float a, float b) { return a + b; }
float Sub(float a, float b) { return a - b; }
float Mul(float a, float b) { return a * b; }
float Div(float a, float b) { return a / b; }
float Sqrt(float a) { return sqrt(a); }
#endif

#endif