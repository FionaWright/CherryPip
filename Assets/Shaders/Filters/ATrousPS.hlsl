#include "CBV.h"

struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

ConstantBuffer<CbvFilterATrous> c_atrous : register(b0);

Texture2D<float4>    gColor  : register(t0);
Texture2D<float4>    gNormalsDepth  : register(t1);
SamplerState gSampler : register(s0);

float3 ReconstructWorldPosition(float2 uv, float depth)
{
    float4 ndc;
    ndc.xy = uv * 2.0f - 1.0f;
    ndc.z  = depth;
    ndc.w  = 1.0f;

    float4 world = mul(ndc, c_atrous.InvVP);
    return world.xyz / world.w;
}

float3 SampleWorldPos(float2 uv)
{
	float depthSample = gNormalsDepth.Sample(gSampler, uv).a;
	return ReconstructWorldPosition(uv, depthSample);
}

// https://jo.dreggn.org/home/2010_atrous.pdf
float4 PSMain(VsOut input) : SV_Target
{
    float KernelB3Weights[25] = {
        0.00391, 0.01563, 0.02344, 0.01563, 0.00391,
        0.01563, 0.06250, 0.09375, 0.06250, 0.01563,
        0.02344, 0.09375, 0.14063, 0.09375, 0.02344,
        0.01563, 0.06250, 0.09375, 0.06250, 0.01563,
        0.00391, 0.01563, 0.02344, 0.01563, 0.00391
    };

    int2 KernelOffsets[25] = {
        int2(-2,-2), int2(-1,-2), int2(0,-2), int2(1,-2), int2(2,-2),
        int2(-2,-1), int2(-1,-1), int2(0,-1), int2(1,-1), int2(2,-1),
        int2(-2, 0), int2(-1, 0), int2(0, 0), int2(1, 0), int2(2, 0),
        int2(-2, 1), int2(-1, 1), int2(0, 1), int2(1, 1), int2(2, 1),
        int2(-2, 2), int2(-1, 2), int2(0, 2), int2(1, 2), int2(2, 2)
    };

    float3 valC = gColor.Sample(gSampler, input.uv).rgb;
    float3 valN = gNormalsDepth.Sample(gSampler, input.uv).rgb;
    float3 valP = SampleWorldPos(input.uv);

    float cumW = 0.0f;
    float3 sum = 0.0f;

    [unroll]
    for (int i = 0; i < 25; i++)
    {
        float2 uv = input.uv + KernelOffsets[i] * c_atrous.StepWidth * c_atrous.TexelSize;
        float stepWidth2 = c_atrous.StepWidth * c_atrous.StepWidth;

        float3 sampleC = gColor.Sample(gSampler, uv).rgb;
        float3 tC = valC - sampleC;
        float distC2 = dot(tC, tC);
        float wC = min(1.0f, exp(-distC2/c_atrous.phiC));

        float3 sampleN = gNormalsDepth.Sample(gSampler, uv).rgb;
        float3 tN = valN - sampleN;
        float distN2 = dot(tN, tN);
        float wN = min(1.0f, exp(-distN2/c_atrous.phiN));

        float3 sampleP = SampleWorldPos(uv);
        float3 tP = valP - sampleP;
        float distP2 = max(0.0f, dot(tN, tN) / stepWidth2);
        float wP = min(1.0f, exp(-distP2/c_atrous.phiP));

        float weight = wC * wN * wP;
        sum += sampleC * weight * KernelB3Weights[i];
        cumW += weight * KernelB3Weights[i];
    }

    return float4(sum / cumW, 1);
}