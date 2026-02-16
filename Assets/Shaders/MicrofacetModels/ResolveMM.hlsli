#ifndef H_MICRO_CLASS_DEF_H
#define H_MICRO_CLASS_DEF_H

struct MicrofacetModel;

#ifdef NDF_TYPE_GGX
#    include "MicrofacetModels/MM_GGX_Smith_Iso.hlsli"
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

#endif