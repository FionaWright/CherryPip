#include "CBV.h"
#include "MathUtils.hlsli"
#include "ShadingFrame.hlsli"

struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : TEXCOORD1;
};

Texture2D<float4> gTextures[] : register(t0);

SamplerState gSampler : register(s0);

ConstantBuffer<CbvRasterMaterial> c_mat : register(b1);

struct GBufferOut
{
    float4 RgbaAlbedo : SV_Target0;
    float4 RgbNormal_ADepth : SV_Target1;
	float4 RRough_GMetallic : SV_Target2; // 1 Reserved
	float4 RgbEmissive : SV_Target3; // 1 Reserved
};

GBufferOut PSMain(VsOut input)
{
    GBufferOut output;

    output.RgbaAlbedo = gTextures[c_mat.TexIdxAlbedo].Sample(gSampler, input.uv).rgba;

	float3 bumpSample = gTextures[c_mat.TexIdxNormal].SampleLevel(gSampler, input.uv, 0).rgb * 2.0f - 1.0f;
	bumpSample.y = -bumpSample.y; // DX convention

    float3 N = normalize(input.normal);
    ShadingFrame bumpFrame = CreateShadingFrame(N);
	float3 N_w = bumpFrame.ToWorld(bumpSample);

    output.RgbNormal_ADepth.rgb = N_w * 0.5f + 0.5f; // [-1,1] -> [0,1]
    output.RgbNormal_ADepth.a = input.position.z / input.position.w;

	output.RRough_GMetallic.rg = gTextures[c_mat.TexIdxRoughMet].Sample(gSampler, input.uv).gb;
    output.RRough_GMetallic.ba = 0.0f;

	output.RgbEmissive.rgb = gTextures[c_mat.TexIdxEmissive].Sample(gSampler, input.uv).rgb;
	output.RgbEmissive.a = 0.0f;

    return output;
}