#ifndef H_MICRO_CLASS_DEF_H
#define H_MICRO_CLASS_DEF_H

struct MicrofacetModel;

// Missing:
// VNDF Aniso

#if defined(NDF_TYPE_GGX)
#   ifdef ANISOTROPY_ENABLED
#       include "MicrofacetModels/MM_GGX_Smith_Aniso.hlsli"
#   else
#       ifdef SAMPLE_VISIBLE_NORMALS
#           if defined(MASKING_VCAVITY)
#               include "MicrofacetModels/MM_GGX_VCavity_Iso_VNDF.hlsli"
#           elif defined(MASKING_SMITH)
#               include "MicrofacetModels/MM_GGX_Smith_Iso_VNDF.hlsli"
#           endif
#       else
#           include "MicrofacetModels/MM_GGX_Smith_Iso.hlsli"
#       endif
#   endif
#elif defined(NDF_TYPE_BECKMANN)
#    include "MicrofacetModels/MM_Beckmann_Smith_Iso.hlsli"
#endif

#ifndef MICROFACET_MODEL_CHOSEN
#	include "MicrofacetModels/MM_GGX_Smith_Iso.hlsli"
#endif

void InitializeMM(
    inout MicrofacetModel mm,
    float roughness,
    RngInfo rngInfo, // For computing U3
    float3 V
)
{
#ifdef NDF_TYPE_GGX
#   ifdef SAMPLE_VISIBLE_NORMALS
    mm.Init(roughness, rngInfo, V);
#   else
    mm.Init(roughness);
#   endif
#endif
}

void InitializeMMAniso(
    inout MicrofacetModel mm,
    float3 T, float3 B, float3 N,
    float3 anisoDirAndStrength
)
{
#ifdef ANISOTROPY_ENABLED
    mm.InitAniso(T, B, N, anisoDirAndStrength);
#endif
}

#endif