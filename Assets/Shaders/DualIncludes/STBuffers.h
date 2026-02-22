#ifndef H_STBUFFERS_H
#define H_STBUFFERS_H

#include "HlslGlue.h"
#include "Spectrum.h"

struct StInstanceData
{
    uint IndexBufferOffset;
    uint VertexBufferOffset;
    uint MaterialIdx;
    uint p;

    float4x4 M;
    float4x4 MTI;
};

enum StMaterialFlags : uint
{
    eNone = 0,
    eIsGlass = 1
};

struct StMaterialData
{
    float3 BaseColorFactor;
//    float EmissiveStrength;
//
//    float3 EmissiveColor;
//    uint TexIdxAlbedo;
//
//    uint TexIdxNormal;
//    uint TexIdxRoughMet;
//    uint TexIdxEmissive;
//    uint TexIdxAniso;
//
//    float Roughness;
//    float Metalness;
//    float DiffuseProbability;
//    float IoR;
//
//    StMaterialFlags Flags;
//    float AnisoStrength;
//    float2 p2;
//
//    float3 GlassSigmaA;
//    float p;
};

struct Vertex
{
    float3 position;
    float2 uv;
    float3 normal;
};

#endif