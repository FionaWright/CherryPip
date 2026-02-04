#include "Buffers.h"
#include "Cbv.h"

Texture2D<float4> gTexA : register(t0);
Texture2D<float4> gTexB : register(t1);
RWStructuredBuffer<SumSquaredErrorStruct> gOutputBuffer : register(u0);
SamplerState gSampler : register(s0);

ConstantBuffer<CbvMaxLumRedSearch> cbv : register(b0); // TODO: rename CBV

#define BLOCK_SIZE 9
#define WARP_SIZE_1D 16
#define WARP_SIZE WARP_SIZE_1D*WARP_SIZE_1D

// 16x16 * 9x9 = 144x144 per thread group
// Total tgroups = 4096x4096/144x144 = 29x29
// DTid is 29x29 * 16x16 in 464x464

// Multiply DTid by 9x9 to get UV in 4096x4096
// Divide DTid.x by 16 to get the buffer idx

groupshared float localSquaredErr[WARP_SIZE];

[numthreads(WARP_SIZE_1D, WARP_SIZE_1D, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint2 threadID = fmod(DTid.xy, WARP_SIZE_1D);
    uint threadIdx = threadID.y * WARP_SIZE_1D + threadID.x;

    // Loop over block of pixels
    // Compute sum (Ai - Bi)^2
    // Write sum to output buffer
    // On CPU:
    // MSE = Sum all structs and divide by N (width x height)
    // Get Sqrt(MSE) to get RMSE

    float squaredErrSum = 0.0f;

    [unroll]
    for (int y = 0; y < BLOCK_SIZE; y++)
        [unroll]
        for (int x = 0; x < BLOCK_SIZE; x++)
        {
            float2 uvXY = float2(DTid.xy * BLOCK_SIZE) + float2(x, y);
            float2 uv = (uvXY + 0.5f) * cbv.TexelSize; // Center in pixel

            float3 colorA = gTexA.SampleLevel(gSampler, uv, 0).rgb;
            float3 colorB = gTexB.SampleLevel(gSampler, uv, 0).rgb;
            float lumA = dot(colorA, float3(0.2126,0.7152,0.0722));
            float lumB = dot(colorB, float3(0.2126,0.7152,0.0722));

            squaredErrSum += (lumA - lumB) * (lumA - lumB);
        }

    localSquaredErr[threadIdx] = squaredErrSum;

    GroupMemoryBarrierWithGroupSync();
    if (threadIdx != 0)
        return;

    for (int i = 1; i < WARP_SIZE; i++)
    {
        squaredErrSum += localSquaredErr[i];
    }

    uint2 groupID = DTid.xy / WARP_SIZE_1D;
    uint outputIdx = groupID.y * 29 + groupID.x;

    SumSquaredErrorStruct output = { squaredErrSum };
    gOutputBuffer[outputIdx] = output;
}