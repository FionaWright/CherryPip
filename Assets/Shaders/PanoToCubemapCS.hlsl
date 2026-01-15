Texture2D<float3> gPano : register(t0);
RWTexture2DArray<float4> gCubemap : register(u0);

SamplerState gSampler : register(s0);

cbuffer CB : register(b0)
{
    uint gOutputWidth; // Width=Height
    uint gInputWidth;
    uint gInputHeight;
    float gRotation;
}

#define PI 3.141592653589793

float3 UvToDir(uint face, float2 uv)
{
    float3 d;
    switch (face)
    {
    case 0: // +X
        d = float3(1.0, -uv.y, -uv.x);
        break;
    case 1: // -X
        d = float3(-1.0, -uv.y, uv.x);
        break;
    case 2: // +Y
        d = float3(uv.x, 1.0, uv.y);
        break;
    case 3: // -Y
        d = float3(uv.x, -1.0, -uv.y);
        break;
    case 4: // +Z
        d = float3(uv.x, -uv.y, 1.0);
        break;
    case 5: // -Z
        d = float3(-uv.x, -uv.y, -1.0);
        break;
    default:
        return 0;
    }

    return normalize(d);
}

float2 PanoSphereToSquare(float3 d)
{
    float lambda = atan2(d.z, d.x);    // [-pi,pi]
    float phi = asin(clamp(d.y, -1.0f, 1.0f));
    float u = (lambda + PI) / (2.0f * PI);
    float v = (phi + 0.5 * PI) / PI;
    u = frac(u);
    return float2(u, v);
}

[numthreads(16,16,1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x >= gOutputWidth || DTid.y >= gOutputWidth)
        return;

    uint face = DTid.z;
    float2 uv = (float2(DTid.xy) / float2(gOutputWidth.xx - 1));
    uv = uv * 2.0f - 1.0f; // [-1, 1]

    float3 dir = UvToDir(face, uv);
    float2 panoUV = PanoSphereToSquare(dir);
    panoUV.y = 1 - panoUV.y;

    float3 color = gPano.SampleLevel(gSampler, panoUV, 0.0f).rgb;

    gCubemap[DTid] = float4(color, 1);
}
