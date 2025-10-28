#ifndef H_HLSLGLUE_H
#define H_HLSLGLUE_H

#ifdef __cplusplus

#include <DirectXMath.h>
typedef DirectX::XMMATRIX float4x4;
typedef DirectX::XMFLOAT2 float2;
typedef DirectX::XMFLOAT3 float3;
typedef DirectX::XMFLOAT4 float4;
typedef uint32_t uint;
typedef struct uint2 { uint32_t x; uint32_t y; } uint2;
typedef struct uint3 { uint32_t x; uint32_t y; uint32_t z; } uint3;
typedef struct uint4 { uint32_t x; uint32_t y; uint32_t z; uint32_t w; } uint4;

#endif

#endif