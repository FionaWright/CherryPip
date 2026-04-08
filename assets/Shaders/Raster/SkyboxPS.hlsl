struct VsOut
{
    float4 Position : SV_Position;
    float3 ViewDirection : TEXCOORD0;
};

TextureCube gCubemap : register(t0);
SamplerState gSampler : register(s0);

float4 PSMain(VsOut input) : SV_Target
{
    float3 col = gCubemap.SampleLevel(gSampler, input.ViewDirection, 1).rgb;
    return float4(col, 1);
}