Texture2D<float3> gPano : register(t0);
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

// Concentric square→disk (Shirley–Chiu)
float2 SquareToDiskConcentric(float2 a)
{
    // a ∈ [-1,1]^2
    if (a.x == 0 && a.y == 0)
        return float2(0, 0);

    float r, theta;
    float ax = abs(a.x);
    float ay = abs(a.y);

    if (ax > ay)
    {
        r = ax;
        theta = (PI * 0.25f) * (a.y / a.x);
    }
    else
    {
        r = ay;
        theta = (PI * 0.5f) - (PI * 0.25f) * (a.x / a.y);
    }

    float cs = cos(theta);
    float sn = sin(theta);
    return r * float2(cs, sn);
}

float3 EaUvToDir(float2 uv)
{
    // 1) uv → [-1,1]
    float2 a = uv * 2.0f - 1.0f;

    // 2) square → disk
    float2 d = SquareToDiskConcentric(a);

    // 3) scale to Lambert disk coordinates (radius ≤ 2)
    float2 X = d * 2.0f;

    // 4) inverse Lambert azimuthal equal-area projection
    float rr = dot(X, X);            // X^2 + Y^2
    float s  = max(0.0f, 1.0f - 0.25f * rr);
    float k  = sqrt(s);

    float3 dir;
    dir.x = k * X.x;
    dir.y = -1.0f + 0.5f * rr;       // south pole at disk center
    dir.z = k * X.y;

    return normalize(dir);
}

float2 DirToPanoUv(float3 d)
{
    float lambda = atan2(d.z, d.x);    // [-pi,pi]
    float phi = asin(clamp(d.y, -1.0f, 1.0f));
    float u = (lambda + PI) / (2.0f * PI);
    float v = (phi + 0.5 * PI) / PI;
    u = frac(u);
    return float2(u, v);
}

[numthreads(16,16,1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= gOutputWidth || dispatchThreadID.y >= gOutputHeight)
        return;

    // Normalized coordinates in [0,1]
    float u = (dispatchThreadID.x + 0.5f) / gOutputWidth;
    float v = (dispatchThreadID.y + 0.5f) / gOutputHeight;

    float3 dir = EaUvToDir(float2(u, 1 - v));
    float2 panoUV = DirToPanoUv(dir);

    float3 color = gPano.SampleLevel(gSampler, panoUV, 0.0f).rgb;

    gEA[dispatchThreadID.xy] = float4(color, 1);
}
