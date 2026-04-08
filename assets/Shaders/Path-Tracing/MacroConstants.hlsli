#ifndef H_MACRO_CONSTANTS_H
#define H_MACRO_CONSTANTS_H

// Converts most macros to static constants as they make code much cleaner

enum RngSamplingStrategy
{
	eInvalidRngSampling,
	eIndependent,
	eHalton,
	eHaltonApple,
	eOwenScrambledHalton
};

#if defined(SAMPLING_INDEPENDENT)
static const RngSamplingStrategy cRngSamplingStrategy = eIndependent;
#elif defined(SAMPLING_HALTON)
static const RngSamplingStrategy cRngSamplingStrategy = eHalton;
#elif defined(SAMPLING_HALTON_APPLE)
static const RngSamplingStrategy cRngSamplingStrategy = eHaltonApple;
#elif defined(SAMPLING_HALTON_OWEN)
static const RngSamplingStrategy cRngSamplingStrategy = eOwenScrambledHalton;
#else
static const RngSamplingStrategy cRngSamplingStrategy = eInvalidRngSampling;
#endif

enum LightingModel
{
    eInvalidLightingModel,
    eLambert,
    eGlossy,
    eMicrofacet
};

#if defined(LIGHTING_LAMB_DIFF)
static const LightingModel cLightingModel = eLambert;
#elif defined(LIGHTING_GLOSSY)
static const LightingModel cLightingModel = eGlossy;
#elif defined(LIGHTING_MICROFACET)
static const LightingModel cLightingModel = eMicrofacet;
#else
static const LightingModel cLightingModel = eInvalidLightingModel;
#endif

#ifdef JITTER_ENABLED
static const bool cJitterEnabled = true;
#else
static const bool cJitterEnabled = false;
#endif

#ifdef DEPTH_OF_FIELD_ENABLED
static const bool cDofEnabled = true;
#else
static const bool cDofEnabled = false;
#endif

#ifdef DEBUG_PT_INFO_OUTPUT
static const bool cDebugInfoOutputEnabled = true;
#else
static const bool cDebugInfoOutputEnabled = false;
#endif

#ifdef DEBUG_PATH_VISUALIZATION
static const bool cDebugPathVisualizationEnabled = true;
#else
static const bool cDebugPathVisualizationEnabled = false;
#endif

#ifdef DIR_LIGHT_ENABLED
static const bool cDirLightEnabled = true;
#else
static const bool cDirLightEnabled = false;
#endif

#ifdef DIR_LIGHT_DISTANT
static const bool cDirLightIsDistant = true;
#else
static const bool cDirLightIsDistant = false;
#endif

#ifdef ENV_MAP_ENABLED
static const bool cEnvMapEnabled = true;
#else
static const bool cEnvMapEnabled = false;
#endif

#ifdef ENV_MAP_EA
static const bool cEnvMapIsEqualArea = true;
#else
static const bool cEnvMapIsEqualArea = false;
#endif

#ifdef FURNACE_TEST_HEMI_HEMI_EMIT
static const bool cFurnaceTestHHE = true;
#else
static const bool cFurnaceTestHHE = false;
#endif

#ifdef FURNACE_TEST_HEMI_DIR_REFLECT
static const bool cFurnaceTestHDR = true;
#else
static const bool cFurnaceTestHDR = false;
#endif

#ifdef ANISOTROPY_ENABLED
static const bool cAnisotropyEnabled = true;
#else
static const bool cAnisotropyEnabled = false;
#endif

#ifdef ALPHA_TESTING_ENABLED
static const bool cAlphaTestingEnabled = true;
#else
static const bool cAlphaTestingEnabled = false;
#endif

#ifdef RUSSIAN_ROULETTE_ENABLED
static const bool cRussianRouletteEnabled = true;
#else
static const bool cRussianRouletteEnabled = false;
#endif

#ifdef NORMAL_MAPS_ENABLED
static const bool cNormalMapsEnabled = true;
#else
static const bool cNormalMapsEnabled = false;
#endif

#ifdef IMPORTANCE_SAMPLING
static const bool cImportanceSamplingEnabled = true;
#else
static const bool cImportanceSamplingEnabled = false;
#endif

#ifdef DEBUG_FORCE_SPECULAR
static const bool cDebugForceSpecular = true;
#else
static const bool cDebugForceSpecular = false;
#endif

#ifdef DEBUG_FORCE_DIFFUSE
static const bool cDebugForceDiffuse = true;
#else
static const bool cDebugForceDiffuse = false;
#endif

#ifdef DEBUG_FORCE_REFLECT
static const bool cDebugForceReflect = true;
#else
static const bool cDebugForceReflect = false;
#endif

#ifdef DEBUG_FORCE_REFRACT
static const bool cDebugForceRefract = true;
#else
static const bool cDebugForceRefract = false;
#endif

#ifdef LIGHTING_GLASS_ENABLED
static const bool cLightingGlassEnabled = true;
#else
static const bool cLightingGlassEnabled = false;
#endif

#ifdef GAMMA_CORRECTION
static const bool cGammaCorrection = true;
#else
static const bool cGammaCorrection = false;
#endif

// Avoid code breaking
#if !defined(NDF_TYPE_GGX) && !defined(NDF_TYPE_BECKMANN)
#define NDF_TYPE_GGX
#endif

#endif