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

#include <algorithm>
#include <cmath>
#define glueMax std::max
#define glueAbs std::abs
#define glueSqrt std::sqrt
#define glueCos std::cos
#define glueSin std::sin
#define glueAtan2 std::atan2
#define glueAsin std::asin

inline float glueClamp(const float x, const float xmin, const float xmax) { return std::max(xmin, std::min(xmax, x)); }
inline float glueFrac(const float x) { return std::fmod(x, 1.0f); }

#else

#define glueMax max
#define glueAbs abs
#define glueSqrt sqrt
#define glueCos cos
#define glueSin sin
#define glueAtan2 atan2
#define glueAsin asin
#define glueClamp clamp
#define glueFrac frac

#define const

#endif

#endif