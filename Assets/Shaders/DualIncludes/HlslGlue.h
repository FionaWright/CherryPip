#ifndef H_HLSLGLUE_H
#define H_HLSLGLUE_H

#ifdef __cplusplus

#include <DirectXMath.h>
typedef DirectX::XMFLOAT4X4 float4x4;
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
#define glueMin std::min
#define glueAbs std::abs
#define glueSqrt std::sqrt
#define glueCos std::cos
#define glueSin std::sin
#define glueAtan std::atan
#define glueAtan2 std::atan2
#define glueAsin std::asin

inline float glueClamp(const float x, const float xmin, const float xmax) { return std::max(xmin, std::min(xmax, x)); }
inline float glueFrac(const float x) { return std::fmod(x, 1.0f); }
inline float3 glueNormalize(const float3 v)
{
    const float mag = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return {v.x / mag, v.y / mag, v.z / mag};
}
inline float glueDot(const float3 a, const float3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

#else

#define glueMax max
#define glueMin min
#define glueAbs abs
#define glueSqrt sqrt
#define glueCos cos
#define glueSin sin
#define glueAtan atan
#define glueAtan2 atan2
#define glueAsin asin
#define glueClamp clamp
#define glueFrac frac
#define glueNormalize normalize
#define glueDot dot

#define const

#endif

#endif