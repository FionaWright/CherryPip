#ifndef H_MICRO_CLASS_DEF_H
#define H_MICRO_CLASS_DEF_H

struct MicrofacetModel;

#ifdef NDF_TYPE_GGX
#   ifdef ANISOTROPY_ENABLED
#       include "MicrofacetModels/MM_GGX_Smith_Aniso.hlsli"
#   else
#       include "MicrofacetModels/MM_GGX_Smith_Iso.hlsli"
#   endif
#endif

void InitializeMM(
    inout MicrofacetModel mm,
    float roughness,
    RngInfo rngInfo // For computing U3
)
{
#ifdef NDF_TYPE_GGX
    mm.Init(roughness);
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