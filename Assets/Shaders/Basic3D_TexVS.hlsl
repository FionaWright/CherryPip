struct VsIn
{
    float3 position : POSITION;
    float4 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct VsOut
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct CbvMatrices
{
    float4x4 M; // Model
    float4x4 MTI; // Model Transpose Inverse (For Normals)
    float4x4 V; // View
    float4x4 P; // Projection
};
ConstantBuffer<CbvMatrices> c_matrices : register(b0);

VsOut VSMain(VsIn input)
{
    VsOut output;

    float4 pos = float4(input.position, 1.0f);
    pos = mul(c_matrices.M, pos);

    output.normal = normalize(mul((float3x3)c_matrices.MTI, (float3)input.normal));

    pos = mul(c_matrices.V, pos);

    output.position = mul(c_matrices.P, pos);

    return output;
}