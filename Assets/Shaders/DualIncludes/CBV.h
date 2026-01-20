#ifndef H_CBV_H
#define H_CBV_H

#include "HlslGlue.h"

struct CbvMatrices
{
    float4x4 M; // Model
    float4x4 MTI; // Model Transpose Inverse (For Normals)
    float4x4 V; // View
    float4x4 P; // Projection
};

enum RasterDebugMode
{
    ePosition,
    eNormalsVertex,
    eNormalsBumped,
    eTangent,
    eBinormal,
    eUV,
    eDirLighting,
    eTex,
    eDirLightingTex,
    eRoughness,
    eMetalness,
    eEmission,
    eViewDir,
    eFresnel,
    eReflection,
    eIrradianceIBL,
    eMicrofacetSpecular,
    eMicrofacetDiffuse,
    eMicrofacetIndirect,
    eMicrofacetLo,
    eMicrofacetLoWithIndirect,
};

struct CbvRasterDebug
{
    RasterDebugMode Mode;
    float3 DirLightDir;

    uint MaxCubemapMipMaps;
    float3 p;
};

struct CbvDeferredLighting
{
    float4x4 InvP;

    float3 DirLightDir;
    float p;
};

struct CbvRasterVS
{
    float3 CameraPos;
    float p;
};

struct CbvPathTracing
{
    float4x4 InvP;
    float4x4 InvV;
    float3 CameraPositionWorld;
    uint NumBounces;

    uint RussianRouletteMinBounces;
    uint SPP;
    uint NumFrames;
    uint AccumulationEnabled;

    uint WindowAppGuiWidth;
    uint UpdateAccumulation;
    float2 Jitter;

    float3 DirLight;
    float DirLightCosAngularRadius;

    float3 DirLightColor;
    float DirLightIntensity;
};

enum class DebugBuffer
{
    eNormalsShaded,
    eNormalsGeo,
    eBaseColor,
    eHitPos,
    eFirstBounceDirection,
    eMissHit,
    eHitDistRay0,
    eHitDistRay1,
    eMaterialID,
    eRNG,
    eSelfIntersection,
    eNaN,
    eRoughness,
    eMetalness,

    // Unimplemented for now:
    eDirectLighting,
    eIndirectLighting,
};

struct CbvPathTracingDebug
{
    DebugBuffer DebugIdx;
    uint3 p;
};

struct CbvHighlightPixel
{
    uint2 SelectedPixelCoords;
    uint2 p;
};

struct CbvFilterBoxAndGauss
{
    float2 TexelSize;
    int Radius;      // box radius (e.g. 1 = 3x3, 2 = 5x5)
    float p;
};

struct CbvFilterATrous
{
    uint StepWidth;
    float2 TexelSize;
    float phiC;

    float phiN;
    float phiP;
    float2 p;

    float4x4 InvVP;
};

struct CbvMaxLumRedSearch
{
    float2 TexelSize;
    float2 p;
};

#endif