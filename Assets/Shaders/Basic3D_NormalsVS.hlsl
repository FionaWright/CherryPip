struct VsIn
{
    float3 position : POSITION;
    float3 normal : NORMAL;
}

struct VsOut
{
    float4 position : SV_POSITION;
    float3 normal : SV_NORMAL;
};

struct CbvMatrices
{
    float4x4 M;
    float3x3 MTI;
    float4x4 V;
    float4x4 P;
};
ConstantBuffer<CbvMatrices> c_matrices : register(b0);

PSInput VSMain(VsIn input)
{
    VsOut output;

    float4 pos = float4(input.position, 1.0f);
    pos = mul(c_matrices.M, pos);

    output.normal = normalize(mul(c_matrices.MTI, pos));

    pos = mul(c_matrices.V, pos);

    output.position = mul(c_matrices.P, pos);

    return output;
}