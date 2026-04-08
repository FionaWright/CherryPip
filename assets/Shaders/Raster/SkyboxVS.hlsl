struct VsIn
{
    float3 Position : POSITION;
};

struct VsOut
{
    float4 Position : SV_Position;
    float3 ViewDirection : TEXCOORD0;
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
    VsOut o;

    float4 pos = float4(input.Position, 1.0f);
    float4x4 V_rot = c_matrices.V;
    V_rot._14 = V_rot._24 = V_rot._34 = 0.0f;
    o.Position = mul(c_matrices.P, mul(V_rot, pos));

    o.Position.z = o.Position.w;

    o.ViewDirection = normalize(input.Position.xyz);

    return o;
}