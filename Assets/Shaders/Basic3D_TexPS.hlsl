struct VsOut
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

Texture2D c_diffuseTex : register(t0);
Sampler c_sampler : register(s0);

float4 PSMain(VsOut input) : SV_TARGET
{
	return c_diffuseTex.Sample(c_sampler, input.uv).rgba;
}