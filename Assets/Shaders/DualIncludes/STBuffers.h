#ifndef H_STBUFFERS_H
#define H_STBUFFERS_H

#include "HlslGlue.h"

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

// Higher is better, consider going up to 64/128
#define NUM_SPECTRUM_SAMPLES 32

struct Spectrum
{
    float Samples[NUM_SPECTRUM_SAMPLES]; // Energy indexed by Wavelength (380nm-780nm)
};

struct StMaterialData
{
    Spectrum BaseColorFactor;
//    float EmissiveStrength;
//
//    Spectrum EmissiveColor;
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