#define SPECTRAL

#include "CBV.h"
#include "PtBuffers.h"
#include "DebugPalette.hlsli"
#include "DebugWindow.hlsli"
#include "MathUtils.hlsli"
#include "Path-Tracing/MacroConstants.hlsli"
#include "Spectral-Tracing/MacroConstants.hlsli"

#include "Spectral-Tracing/Spectrum/Spectrum.hlsli"
#include "Spectral-Tracing/SpectralValue/SpectralValue.hlsli"
#include "Spectral-Tracing/Spectrum/ColorSpectrums.hlsli"
#include "Spectral-Tracing/Spectrum/SpectralUtils.hlsli"
#include "Spectral-Tracing/Spectrum/RgbToSpectrum2019.hlsli"

#define EPSILON 1e-2
#define RAY_FLAGS RAY_FLAG_CULL_NON_OPAQUE|RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES

struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

ConstantBuffer<CbvPathTracing> cbvPathTracing : register(b0);
ConstantBuffer<CbvPathTracingDebug> cbvDebug : register(b1);

RaytracingAccelerationStructure gTLAS : register(t0);
StructuredBuffer<PtInstanceData> gInstances : register(t1);
StructuredBuffer<Vertex> gVertexMegaBuffer : register(t2);
StructuredBuffer<uint3>  gIndexMegaBuffer  : register(t3);
StructuredBuffer<PtMaterialData> gMaterials  : register(t4);
Texture2D<float3> gEnvMap  : register(t5);
Texture2D<float4> gTextures[] : register(t6);

RWTexture2D<float4> gAccum : register(u0);

SamplerState gSampler : register(s0);

#include "Random.h"
#include "Spectral-Tracing/Trace.hlsli"

float4 PSMain(VsOut input) : SV_Target0
{
//#include "Spectral-Tracing/Tests.hlsli"

    RayQuery<RAY_FLAGS> q;

    uint flags = RAY_FLAGS;

    uint instanceMask = 0xFF; // ?

    float3 origin = cbvPathTracing.CameraPositionWorld;

    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = 0;
    ray.TMin = 0.001;
    ray.TMax = 1000.0;

    RngInfo rngInfo;

    float3 colorSum = float3(0,0,0);
    for (uint i = 0; i < cbvPathTracing.SPP; i++)
    {
        rngInfo.SampleIdx = i;
        rngInfo.IndependentRngState = PrngSeed((uint2)input.position.xy, rngInfo.SampleIdx, cbvPathTracing.FrameIdx);

        float2 pixelUV = input.position.xy;
        if (cJitterEnabled)
        {
            float rJitterX = Rand01(DIM_JITTER_X, rngInfo);
            float rJitterY = Rand01(DIM_JITTER_Y, rngInfo);
            float2 jitter = float2(rJitterX, rJitterY) - 0.5f;
            pixelUV += jitter;
        }
        pixelUV *= cbvPathTracing.TexelSize;

        float2 ndc = pixelUV * 2.0f - 1.0f; // [0,1] -> [-1,1]
        ndc.y = -ndc.y;
        float4 clip = float4(ndc, 0, 1); // z=0 for near plane
        float4 view = mul(cbvPathTracing.InvP, clip);
        view /= view.w;
        float4 world = mul(cbvPathTracing.InvV, view);
        ray.Direction = normalize(world.xyz - origin);

        if (cDofEnabled)
        {
            float3 camRight = normalize(float3(cbvPathTracing.InvV[0][0], cbvPathTracing.InvV[1][0], cbvPathTracing.InvV[2][0]));
            float3 camUp = normalize(float3(cbvPathTracing.InvV[0][1], cbvPathTracing.InvV[1][1], cbvPathTracing.InvV[2][1]));
            float3 focalPoint = origin + normalize(ray.Direction) * cbvPathTracing.DofFocalDist;

            float rLensU = Rand01(DIM_LENS_U, rngInfo);
            float rLensV = Rand01(DIM_LENS_V, rngInfo);
            float r = sqrt(rLensU) * cbvPathTracing.DofLensRadius;
            float theta = 2.0 * PI * rLensV;
            float3 lensOffset = r * (camRight * cos(theta) + camUp * sin(theta));
            ray.Origin = origin + lensOffset;
            ray.Direction = normalize(focalPoint - ray.Origin);
        }

        float lambda = SampleVisibleWavelength(rngInfo.IndependentRngState);

        if (cDebugForceWavelength)
            lambda = cDebugForcedWavelength;

        colorSum += Trace(q,
                          flags,
                          instanceMask,
                          ray,
                          lambda,
                          rngInfo);
    }

    colorSum /= float(cbvPathTracing.SPP);

    if (!cbvPathTracing.AccumulationEnabled)
        return float4(colorSum, 1);

    uint2 pixelCoord = uint2(input.position.xy);

    float3 accumColor = gAccum.Load(pixelCoord).rgb;
    if (IsNaN3(accumColor) && false) // Disabled due to false positives
        //return float4(1, 0, 1, 1);
        return float4(PcgRand01(rngInfo.IndependentRngState), PcgRand01(rngInfo.IndependentRngState), PcgRand01(rngInfo.IndependentRngState), 1);

    float3 newSum = accumColor + colorSum;

    float accumFrameCount = (float)cbvPathTracing.FrameIdx;
    float totalFrames = accumFrameCount + 1.0f;

    float3 average = (accumColor * accumFrameCount + colorSum) / totalFrames;

    if (cbvPathTracing.UpdateAccumulation)
        gAccum[pixelCoord].rgb = average;

    if (cGammaCorrection)
        average = LRGB_to_SRGB(average);

    return float4(average, 1);
}