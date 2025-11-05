#define BLOCK_SIZE 8

RWTexture2D<float4> gTex : register(u0);

cbuffer CB : register(b0)
{
    float2 TexelSize;
    uint2 SelectedPixelCoords;
}

[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    [unroll]
    for (int x = -1; x <= 1; x++)
        [unroll]
        for (int y = -1; y <= 1; y++)
        {
            if (x == 0 && y == 0)
                continue;
            gTex[SelectedPixelCoords + int2(x, y)] = float4(1, 0, 1, 1);
        }
}