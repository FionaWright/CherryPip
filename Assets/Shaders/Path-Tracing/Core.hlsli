#include "CBV.h"
#include "PtBuffers.h"
#include "Rand01.hlsli"

#define EPSILON 1e-2
#define RAY_FLAGS RAY_FLAG_CULL_NON_OPAQUE|RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES

struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

ConstantBuffer<CbvPathTracing> c_pathTracing : register(b0);
#ifdef DEBUG_BUFFER
    ConstantBuffer<CbvPathTracingDebug> c_debug : register(b1);
	#include "DebugPalette.hlsli"
    #include "Path-Tracing/MathUtils.hlsli"
#endif
RaytracingAccelerationStructure gTLAS : register(t0);
StructuredBuffer<PtInstanceData> gInstances : register(t1);
StructuredBuffer<Vertex> gVertexMegaBuffer : register(t2);
StructuredBuffer<uint3>  gIndexMegaBuffer  : register(t3);
StructuredBuffer<PtMaterialData> gMaterials  : register(t4);
Texture2D<float3> gEnvMap  : register(t5);
Texture2D<float4> gTextures[] : register(t6);

RWTexture2D<float4> gAccum : register(u0);

SamplerState c_sampler : register(s0);

#include "Trace.hlsli"

float4 PSMain(VsOut input) : SV_Target0
{
    RayQuery<RAY_FLAGS> q;

    uint flags = RAY_FLAGS;

    uint instanceMask = 0xFF; // ?

    float3 origin = c_pathTracing.CameraPositionWorld;

    float2 ndc = (input.uv + c_pathTracing.Jitter) * 2.0f - 1.0f; // [-1,1] range
    ndc.y = -ndc.y;
    float4 clip = float4(ndc, 0, 1); // z=0 for near plane
    float4 view = mul(c_pathTracing.InvP, clip);
    view /= view.w;
    float4 world = mul(c_pathTracing.InvV, view);
    float3 rayDir = normalize(world.xyz - origin);

    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = rayDir;
    ray.TMin = 0.001;
    ray.TMax = 1000.0;

#ifdef DEPTH_OF_FIELD_ENABLED
    float3 camRight = normalize(c_pathTracing.InvV[0].xyz);  // X column
    float3 camUp    = normalize(c_pathTracing.InvV[1].xyz);  // Y column
    float3 focalPoint = origin + rayDir * c_pathTracing.DofFocalDist;
#endif

    float3 colorSum = float3(0,0,0);
    for (uint i = 0; i < c_pathTracing.SPP; i++)
    {
        uint rngState = PrngSeed((uint2)input.position, i+4648387, c_pathTracing.NumFrames);

#ifdef DEPTH_OF_FIELD_ENABLED
        float r = sqrt(PcgRand01(rngState)) * c_pathTracing.DofLensRadius;
        float theta = 2.0 * PI * PcgRand01(rngState);
        float3 lensOffset = r * (camRight * cos(theta) + camUp * sin(theta));
        ray.Origin = origin + lensOffset;
        ray.Direction = normalize(focalPoint - ray.Origin);
#endif

        colorSum += Trace(q,
                          flags,
                          instanceMask,
                          ray,
                          rngState);
    }

    colorSum /= float(c_pathTracing.SPP);

    if (!c_pathTracing.AccumulationEnabled)
        return float4(colorSum, 1);

    uint2 pixelCoord = uint2(input.position.xy);
    float3 accumColor = gAccum.Load(pixelCoord).rgb;
    if (IsNaN3(accumColor)) return float4(1, 0, 1, 1);

    float3 newSum = accumColor + colorSum;

    float accumFrameCount = (float)c_pathTracing.NumFrames;
    float totalFrames = accumFrameCount + 1.0f;

    float3 average = (accumColor * accumFrameCount + colorSum) / totalFrames;

    if (c_pathTracing.UpdateAccumulation)
        gAccum[pixelCoord].rgb = average;

    return float4(average, 1);
}