#ifndef H_MACRO_CONSTANTS_SPECTRAL_H
#define H_MACRO_CONSTANTS_SPECTRAL_H

#ifdef SINGLE_LAMBDA_RENDERING
static const bool cSingleLambda = true;
#else
static const bool cSingleLambda = false;
#endif

#ifdef DEBUG_FORCE_WAVELENGTH
static const bool cDebugForceWavelength = true;
static const float cDebugForcedWavelength = DEBUG_FORCED_WAVELENGTH;
#else
static const bool cDebugForceWavelength = false;
static const float cDebugForcedWavelength = -1;
#endif

#endif