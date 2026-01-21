struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 binormal : TEXCOORD3;
};

Texture2D<float4> gAlbedo : register(t0);
Texture2D<float4> gNormal : register(t1);
Texture2D<float4> gRoughMet : register(t2);

SamplerState gSampler : register(s0);

struct GBufferOut
{
    float4 RgbaAlbedo : SV_Target0;
    float4 RgbNormal_ADepth : SV_Target1;
	float4 RgRoughMet : SV_Target2;
};

GBufferOut PSMain(VsOut input)
{
    GBufferOut output;

    output.RgbaAlbedo = gAlbedo.Sample(gSampler, input.uv).rgba;
    output.RgbNormal_ADepth.rgb = normalize(input.normal) * 0.5f + 0.5f; // [-1,1] -> [0,1]
    output.RgbNormal_ADepth.a = input.position.z / input.position.w;
	output.RgRoughMet.rg = gRoughMet.Sample(gSampler, input.uv).gb;
	output.RgRoughMet.ba = 0;

    return output;
}