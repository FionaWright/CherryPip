struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 binormal : TEXCOORD3;
};

Texture2D<float4> gAlbedo : register(t0);
// TODO: gNormal Buffer
// TODO: SpecularRoughness Buffer
SamplerState gSampler : register(s0);

struct GBufferOut
{
    float4 RgbaAlbedo : SV_Target0;
    float4 RgbNormal_AReserved : SV_Target1;
};

GBufferOut PSMain(VsOut input)
{
    GBufferOut output;

    output.RgbaAlbedo = gAlbedo.Sample(gSampler, input.uv).rgba;
    output.RgbNormal_AReserved.xyz = normalize(input.normal);
    output.RgbNormal_AReserved.w = 1.0f;

    return output;
}