struct VsIn
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VsOut VSMain(VsIn input)
{
    VsOut output;
    output.position = float4(input.position, 1.0f);
    output.uv = input.uv;
    return output;
}