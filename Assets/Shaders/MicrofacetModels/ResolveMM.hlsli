#ifndef H_MICRO_CLASS_DEF_H
#define H_MICRO_CLASS_DEF_H

struct MicrofacetModel;

#ifdef NDF_TYPE_GGX
#   ifdef ANISOTROPY_ENABLED
#       include "MicrofacetModels/MM_GGX_Smith_Aniso.hlsli"
#   else
#       ifdef SAMPLE_VISIBLE_NORMALS
#           include "MicrofacetModels/MM_GGX_VCavity_Iso_VNDF.hlsli"
#       else
#           include "MicrofacetModels/MM_GGX_Smith_Iso.hlsli"
#       endif
#   endif
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

#ifdef ANISOTROPY_ENABLED
void InitializeMMAniso(
    inout MicrofacetModel mm,
    float3 T, float3 B, float3 N,
    float3 anisoDirAndStrength
)
{
    mm.InitAniso(T, B, N, anisoDirAndStrength);
}
#endif

#endif