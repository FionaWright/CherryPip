#include "DualIncludes/Cbv.h"

RWTexture2D<float4> gTex : register(u0);
ConstantBuffer<CbvHighlightPixel> cbv : register(b0);

[numthreads(5, 5, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x == 2 && DTid.y == 2)
        return;

    int2 offset = (int2)DTid.xy - int2(2, 2);

    gTex[cbv.SelectedPixelCoords + offset] = float4(1, 0, 1, 1);
}