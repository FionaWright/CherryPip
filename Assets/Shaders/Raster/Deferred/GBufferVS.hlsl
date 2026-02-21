#include "DualIncludes/Cbv.h"

struct VsIn
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
};

struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : TEXCOORD1;
};

ConstantBuffer<CbvMatrices> c_matrices : register(b0);

VsOut VSMain(VsIn input)
{
    VsOut output;

    float4 pos = float4(input.position, 1.0f);
    pos = mul(c_matrices.M, pos);
    pos = mul(c_matrices.V, pos);
    output.position = mul(c_matrices.P, pos);

    output.uv = input.uv;

    output.normal = normalize(mul((float3x3)c_matrices.MTI, (float3)input.normal));
    //output.tangent = normalize(mul((float3x3)c_matrices.MTI, (float3)input.tangent));
    //output.binormal = normalize(mul((float3x3)c_matrices.MTI, (float3)input.binormal));

    return output;
}