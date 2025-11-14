Texture2D<float4> gPano : register(t0);
RWTexture2D<float4> gEA : register(u0);

SamplerState gSampler : register(s0);

cbuffer CB : register(b0)
{
    uint gOutputWidth;
    uint gOutputHeight;
    uint gInputWidth;
    uint gInputHeight;

    float gRotation;
    float3 p;
}

#define PI 3.141592653589793

[numthreads(16,16,1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= gOutputWidth || dispatchThreadID.y >= gOutputHeight)
        return;

    // Normalized coordinates in [0,1]
    float u = (dispatchThreadID.x + 0.5f) / gOutputWidth;
    float v = (dispatchThreadID.y + 0.5f) / gOutputHeight;

    // Map to equal-area sphere coordinates
    // Using Lambert cylindrical equal-area projection
    float theta = 2.0f * PI * u - PI;                    // longitude [-π, π]
    float phi = asin(2.0f * v - 1.0f);                   // latitude [-π/2, π/2]

    // Map spherical coords to panorama texture coordinates
    float panoU = (theta + PI) / (2.0f * PI);
    float panoV = (phi + PI * 0.5f) / PI;

    float4 color = gPano.SampleLevel(gSampler, float2(panoU, panoV), 0.0f);

    gEA[dispatchThreadID.xy] = color;
}
