#include "CBV.h"
#include "PtBuffers.h"
#include "DebugPalette.hlsli"
#include "MathUtils.hlsli"
#include "Path-Tracing/MacroConstants.hlsli"

#include "Path-Tracing/Spectral-Tracing/Spectrum.hlsli"
#include "Path-Tracing/Spectral-Tracing/SpectralUtils.hlsli"
#include "Path-Tracing/Spectral-Tracing/RgbToSpectrum.hlsli"

#define EPSILON 1e-2
#define RAY_FLAGS RAY_FLAG_CULL_NON_OPAQUE|RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES

struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

ConstantBuffer<CbvPathTracing> cbvPathTracing : register(b0);
ConstantBuffer<CbvPathTracingDebug> cbvDebug : register(b1);

#include "Random.h"

RaytracingAccelerationStructure gTLAS : register(t0);
StructuredBuffer<PtInstanceData> gInstances : register(t1);
StructuredBuffer<Vertex> gVertexMegaBuffer : register(t2);
StructuredBuffer<uint3>  gIndexMegaBuffer  : register(t3);
StructuredBuffer<PtMaterialData> gMaterials  : register(t4);
Texture2D<float3> gEnvMap  : register(t5);
Texture2D<float4> gTextures[] : register(t6);

RWTexture2D<float4> gAccum : register(u0);

SamplerState gSampler : register(s0);

#include "Path-Tracing/Spectral-Tracing/Trace.hlsli"

float4 PSMain(VsOut input) : SV_Target0
{
    // CIE Test
    //Spectrum white = WhiteSpectrum();
    //float lambda = (float)VISIBLE_LIGHT_SPECTRUM_MIN + (saturate(input.uv.x) * (float)VISIBLE_LIGHT_SPECTRUM_SIZE);
    //float3 cie = float3(SampleCIE(lambda, cCIE_X), SampleCIE(lambda, cCIE_Y), SampleCIE(lambda, cCIE_Z));
    //return float4(cie, 1);

    // Spectrum to RGB Test
    //Spectrum s = BlackSpectrum();
    //for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
    //{
    //    float lambda = VISIBLE_LIGHT_SPECTRUM_MIN + (i * SPECTRUM_DELTA_LAMBDA);
    //    bool inRange = lambda >= 380.0f && lambda <= 450.0f;
    //    s.Samples[i] = inRange ? 1.0f : 0.0f;
    //}
    //float3 r = SpectrumToRGB(s);
    //return float4(r, 1);

    // Round-Trip Test
    float3 color = float3(input.uv.y, 0, 0);
    Spectrum s;
    s.InitFromRGB(color, eReflectance);
    float3 r = SpectrumToRGB(s);
    if (input.uv.x < 0.5f)
        r = color;
    if (r.x >= 1.0f)
        r = 1.0f;
    return float4(r, 1);

    RayQuery<RAY_FLAGS> q;

    uint flags = RAY_FLAGS;

    uint instanceMask = 0xFF; // ?

    float3 origin = cbvPathTracing.CameraPositionWorld;

    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = 0;
    ray.TMin = 0.001;
    ray.TMax = 1000.0;

    float3 colorSum = float3(0,0,0);
    for (uint i = 0; i < cbvPathTracing.SPP; i++)
    {
        RngInfo rngInfo;
        rngInfo.SampleIdx = i;
        rngInfo.IndependentRngState = PrngSeed((uint2)input.position.xy, rngInfo.SampleIdx, cbvPathTracing.FrameIdx);

        float2 pixelUV = input.position.xy;
        pixelUV *= cbvPathTracing.TexelSize;

        float2 ndc = pixelUV * 2.0f - 1.0f; // [0,1] -> [-1,1]
        ndc.y = -ndc.y;
        float4 clip = float4(ndc, 0, 1); // z=0 for near plane
        float4 view = mul(cbvPathTracing.InvP, clip);
        view /= view.w;
        float4 world = mul(cbvPathTracing.InvV, view);
        ray.Direction = normalize(world.xyz - origin);

        // TODO:
        //float lambda = SampleVisibleWavelength(rngInfo.IndependentRngState);

        colorSum += Trace(q,
                          flags,
                          instanceMask,
                          ray,
                          rngInfo);
    }

    colorSum /= float(cbvPathTracing.SPP);

    if (!cbvPathTracing.AccumulationEnabled)
        return float4(colorSum, 1);

    uint2 pixelCoord = uint2(input.position.xy);

    float3 accumColor = gAccum.Load(pixelCoord).rgb;
    if (IsNaN3(accumColor))
        return float4(1, 0, 1, 1);

    float3 newSum = accumColor + colorSum;

    float accumFrameCount = (float)cbvPathTracing.FrameIdx;
    float totalFrames = accumFrameCount + 1.0f;

    float3 average = (accumColor * accumFrameCount + colorSum) / totalFrames;

    if (cbvPathTracing.UpdateAccumulation)
        gAccum[pixelCoord].rgb = average;

    if (cGammaCorrection)
        average = pow(average, 1.0f/2.2f);

    return float4(average, 1);
}