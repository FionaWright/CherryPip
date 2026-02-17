struct VsIn
{
    float3 Position : POSITION;
    float3 Color : COLOR;
};

struct VsOut
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR;
};

ConstantBuffer<CbvMatrixVP> c_matrix : register(b0);

VsOut VSMain(VsIn input)
{
    VsOut o;
    o.Position = mul(c_matrix.VP, float4(input.Position, 1.0f));
    o.Color = input.Color;
    return o;
}

float4 PSMain(VsOut input) : SV_TARGET
{
    return float4(input.Color, 1);
}