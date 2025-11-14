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
RaytracingAccelerationStructure gTLAS : register(t0);
StructuredBuffer<PtInstanceData> gInstances : register(t1);
StructuredBuffer<Vertex> gVertexMegaBuffer : register(t2);
StructuredBuffer<uint3>  gIndexMegaBuffer  : register(t3);
StructuredBuffer<PtMaterialData> gMaterials  : register(t4);
Texture2D<float4> gTextures[] : register(t5);
RWTexture2D<float4> gAccum : register(u0);
#ifdef DEBUG_BUFFER
    ConstantBuffer<CbvPathTracingDebug> c_debug : register(b1);

	#include "DebugPalette.hlsli"
#endif

SamplerState c_sampler : register(s0);

#include "Path-Tracing/Hit.hlsli"
#include "Path-Tracing/Miss.hlsli"

float3 Trace(inout RayQuery<RAY_FLAGS> q,
            uint flags,
            uint instanceMask,
            RayDesc ray,
            inout uint rngState
)
{
    float3 color = float3(0, 0, 0);
    float3 throughput = float3(1, 1, 1);

#ifdef DEBUG_BUFFER
    #include "Path-Tracing/DebugBuffersPreTrace.hlsli"
#endif

    for (uint i = 0; i <= c_pathTracing.NumBounces; i++)
    {
        q.TraceRayInline(gTLAS, flags, instanceMask, ray);

        q.Proceed();

        if (q.CommittedStatus() != COMMITTED_TRIANGLE_HIT)
        {
#ifdef DEBUG_BUFFER
    #include "Path-Tracing/DebugBuffersOnMiss.hlsli"
#endif
            color += Miss(ray.Origin, ray.Direction);
            break;
        }

        float3 outMaterialColor = 0, outNg = 0, outNs = 0, outLight = 0;
		PtMaterialData mat;
        Hit(rngState, outMaterialColor, outNg, outNs, outLight, mat, q);

		bool isDiffuse = mat.DiffuseProbability >= PcgRand01(rngState);

        color += throughput * outLight;
        throughput *= lerp(float3(1, 1, 1), outMaterialColor, isDiffuse);

		float3 hitPos = ray.Origin + ray.Direction * q.CommittedRayT();
        ray.Origin = hitPos + outNg * EPSILON;

		float3 diffuseDir = normalize(outNs + RandDirectionSphere(rngState));
		float3 specularDir = ray.Direction - 2 * dot(ray.Direction, outNs) * outNs;
		ray.Direction = lerp(specularDir, diffuseDir, mat.Roughness * isDiffuse);

#ifdef DEBUG_BUFFER
    #include "Path-Tracing/DebugBuffersOnHit.hlsli"
#endif
    }

#ifdef DEBUG_BUFFER
    #include "Path-Tracing/DebugBuffersPostTrace.hlsli"
#endif

    return color;
}

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

    float3 colorSum = float3(0,0,0);
    for (uint i = 0; i < c_pathTracing.SPP; i++)
    {
        uint rngState = PrngSeed((uint2)input.position, i+4648387, c_pathTracing.NumFrames);
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

    float3 newSum = accumColor + colorSum;

    float accumFrameCount = (float)c_pathTracing.NumFrames;
    float totalFrames = accumFrameCount + 1.0f;

    float3 average = (accumColor * accumFrameCount + colorSum) / totalFrames;

    if (c_pathTracing.UpdateAccumulation)
        gAccum[pixelCoord].rgb = average;

    return float4(pow(average, 1/2.2f), 1);
}