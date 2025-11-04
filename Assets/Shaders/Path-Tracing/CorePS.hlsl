#include "CBV.h"
#include "PtBuffers.h"
#include "Rand01.hlsli"

#define EPSILON 1e-4
#define RAY_FLAGS RAY_FLAG_CULL_NON_OPAQUE|RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES

struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

ConstantBuffer<CbvPathTracing> c_pathTracing : register(b0);
RaytracingAccelerationStructure gTLAS : register(t0);
StructuredBuffer<PtInstanceData> gInstances : register(t1);
StructuredBuffer<Vertex> gVertexMegaBuffer : register(t2);
StructuredBuffer<uint3>  gIndexMegaBuffer  : register(t3);
StructuredBuffer<PtMaterialData> gMaterials  : register(t4);
RWTexture2D<float4> gAccum : register(u0);
#ifdef DEBUG_BUFFER
    ConstantBuffer<CbvPathTracingDebug> c_debug : register(b1);
    RWTexture2D<float4> gDebugBuffer : register(u1);
#endif

#include "Path-Tracing/Hit.hlsli"
#include "Path-Tracing/Miss.hlsli"

float3 Trace(RayQuery<RAY_FLAGS> q,
            uint flags,
            uint instanceMask,
            RayDesc ray,
            inout uint rngState
#ifdef DEBUG_BUFFER
            , float2 inputPos
#endif
)
{
    float3 color = float3(0, 0, 0);
    float3 throughput = float3(1, 1, 1);

    for (uint i = 0; i <= c_pathTracing.NumBounces; i++)
    {
        q.TraceRayInline(gTLAS, flags, instanceMask, ray);

        q.Proceed();

        if (q.CommittedStatus() != COMMITTED_TRIANGLE_HIT)
        {
            color += Miss(ray.Origin, ray.Direction);
            break;
        }

        float3 outMaterialColor = 0, outNormal = 0, outLight = 0;
        Hit(rngState, outMaterialColor, outNormal, outLight, q);

        float3 newDir = normalize(outNormal + RandDirectionSphere(rngState));

        color += throughput * outLight;
        throughput *= outMaterialColor;

        float3 hitPos = ray.Origin + ray.Direction * q.CommittedRayT();
        ray.Direction = newDir;
        ray.Origin = hitPos + ray.Direction * max(EPSILON, EPSILON * (float)q.CommittedRayT());

#ifdef DEBUG_BUFFER
        if (i == 0 && c_debug.DebugIdx == DebugBuffer::eNormals)
            gDebugBuffer[inputPos] = float4(outNormal, 1);
        else if (i == 0 && c_debug.DebugIdx == DebugBuffer::eFirstBounceDirection)
            gDebugBuffer[inputPos] = float4(newDir, 1);
        else if (i == 0 && c_debug.DebugIdx == DebugBuffer::eHitPos)
            gDebugBuffer[inputPos] = float4(hitPos, 1);
#endif
    }

    return color;
}

float4 PSMain(VsOut input) : SV_Target0
{
    RayQuery<RAY_FLAGS> q;

    uint flags = RAY_FLAGS|RAY_FLAG_CULL_BACK_FACING_TRIANGLES;

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

    input.position.x -= c_pathTracing.WindowAppGuiWidth;

    float3 colorSum = float3(0,0,0);
    for (uint i = 0; i < c_pathTracing.SPP; i++)
    {
        uint rngState = PrngSeed((uint2)input.position, i+4648387, c_pathTracing.NumFrames);
        colorSum += Trace(q,
                          flags,
                          instanceMask,
                          ray,
                          rngState
#ifdef DEBUG_BUFFER
                          , input.position.xy
#endif
);
    }

    colorSum /= float(c_pathTracing.SPP);
    float4 finalColor = float4(colorSum, 1);

    if (!c_pathTracing.AccumulationEnabled)
        return finalColor;

    float4 accumColor = gAccum.Load(input.position.xy);

    float N = (float)c_pathTracing.NumFrames + 1.0f;
    float4 mu = (finalColor - accumColor) / N;
    mu += accumColor;

    if (c_pathTracing.UpdateAccumulation)
        gAccum[input.position.xy] = mu;
    return gAccum[input.position.xy];
}