Texture2D<float4> gTexA        : register(t0);
//Texture2D<float4> gTexB        : register(t1);

// Each threadgroup writes one float partial to this buffer:
RWStructuredBuffer<float> gPartialSums : register(u0);

// image and dispatch metadata (set as root constants / cbv)
cbuffer Params : register(b0)
{
    uint width;
    uint height;
};

groupshared float sdata[256]; // 16*16

[numthreads(16,16,1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID,
            uint3 groupThreadID    : SV_GroupThreadID,
            uint3 groupID          : SV_GroupID)
{
    uint gx = dispatchThreadID.x;
    uint gy = dispatchThreadID.y;

    uint localIndex = groupThreadID.y * 16 + groupThreadID.x;
    float localSum = 0.0f;

    // Compute for this pixel if in bounds
    if (gx < width && gy < height)
    {
        // load as float4 (if original is RGB, alpha ignored or used depending on channels)
        float4 a = gTexA.Load(int3(gx, gy, 0));
        float4 b = gTexB.Load(int3(gx, gy, 0));
        float4 diff = a - b;

        // squared error across channels:
        float sq = dot(diff, diff);
        localSum = sq;
    } else {
        localSum = 0.0f;
    }

    sdata[localIndex] = localSum;
    GroupMemoryBarrierWithGroupSync();

    // Parallel reduction within the group (power-of-two: 256 -> halves)
    uint stride = 128;
    while (stride > 0)
    {
        if (localIndex < stride)
        {
            sdata[localIndex] += sdata[localIndex + stride];
        }
        GroupMemoryBarrierWithGroupSync();
        stride >>= 1;
    }

    // thread 0 writes the partial sum for this group
    if (localIndex == 0)
    {
        uint groupLinearIndex = groupID.y * ((width + 15) / 16) + groupID.x;
        gPartialSums[groupLinearIndex] = sdata[0];
    }
}
