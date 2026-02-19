#ifndef H_MACRO_CONSTANTS_H
#define H_MACRO_CONSTANTS_H

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

#ifdef LIGHTING_GLASS_ENABLED
static const bool cLightingGlassEnabled = true;
#else
static const bool cLightingGlassEnabled = false;
#endif

#endif