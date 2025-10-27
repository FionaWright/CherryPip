struct VsIn
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
}

struct VsOut
{
    float2 uv : TEXCOORD0;
}

VsOut VSMain(VsIn input)
{
    VsOut output;
    output.uv = input.uv;
    return output;
}