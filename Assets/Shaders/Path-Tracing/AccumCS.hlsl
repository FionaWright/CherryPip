#define BLOCK_SIZE 8

#include "Cbv.h"

ConstantBuffer<CbvAccum> c_accum : register(b0);
Texture2D<float4> gPtOut : register(t0);
RWTexture2D<float4> gAccum : register(u0);
RWTexture2D<float4> gRTV : register(u1);
SamplerState BilinearClamp : register(s0);

[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint n = c_accum.NumFrames;

    float2 texcoords = c_accum.TexelSize * (DTid.xy);
    float4 x_n1 = gPtOut.Sample(BilinearClamp, texcoords);
    float4 mu = gAccum.Load(DTid.xy);
    float4 A = (n * mu + x_n1) / (n + 1);
    gRTV[DTid.xy] = float4(A.xyz, 1);
}