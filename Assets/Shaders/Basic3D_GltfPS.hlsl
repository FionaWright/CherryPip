struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 tangent : TEXCOORD2;
};

Texture2D<float4> c_diffuseTex : register(t0);
SamplerState c_sampler : register(s0);

float4 PSMain(VsOut input) : SV_TARGET
{
    return float4(c_diffuseTex.Sample(c_sampler, input.uv).rgb, 1);
}