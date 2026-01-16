#include "CBV.h"

struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

ConstantBuffer<CbvFilterBoxAndGauss> c_med : register(b0);

Texture2D    gTex  : register(t0);
SamplerState gSampler : register(s0);

void Sort(inout float3 List[9], uint i, uint j)
{
    float3 t = min(List[i], List[j]);
    List[j] = max(List[i], List[j]);
    List[i] = t;
}

void Min(inout float3 List[9], uint i, uint j)
{
    List[i] = min(List[i], List[j]);
}

void Max(inout float3 List[9], uint i, uint j)
{
    List[j] = max(List[i], List[j]);
}

// https://stackoverflow.com/questions/45453537/optimal-9-element-sorting-network-that-reduces-to-an-optimal-median-of-9-network
float3 MedianSelectionNetwork9(inout float3 List[9])
{
    Sort(List, 0,1); Sort(List, 3,4); Sort(List, 6,7);
    Sort(List, 1,2); Sort(List, 4,5); Sort(List, 7,8);
    Sort(List, 0,1); Sort(List, 3,4); Sort(List, 6,7);
    Max(List, 0,3);  Max(List, 3,6);  // (0,3);
    Sort(List, 1,4);  Min(List, 4,7);  Max(List, 1,4);
    Min(List, 5,8);  Min(List, 2,5);  // (5,8);
    Sort(List, 2,4);  Min(List, 4,6);  Max(List, 2,4);
    return List[4];
}

float4 PSMain(VsOut input) : SV_Target
{
    float3 List[9];

    uint k = 0;
    for (float y = -1; y <= 1; ++y)
    {
        for (float x = -1; x <= 1; ++x)
        {
            float2 offset = float2(x, y) * c_med.TexelSize;
            List[k] = gTex.Sample(gSampler, input.uv + offset).rgb;
            k++;
        }
    }

    return float4(MedianSelectionNetwork9(List), 1);
}