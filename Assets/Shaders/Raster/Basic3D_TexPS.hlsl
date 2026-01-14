struct VsOut
{
    float4 position : SV_POSITION;
    float3 normal : TEXCOORD1;
    float2 uv : TEXCOORD0;
};

Texture2D<float4> c_diffuseTex : register(t0);
SamplerState c_sampler : register(s0);

float4 PSMain(VsOut input) : SV_TARGET
{
	return float4(c_diffuseTex.Sample(c_sampler, input.uv).rgb, 1);
}