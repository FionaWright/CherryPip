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

float CopySign(float mag, float sign)
{
	return sign < 0.0f ? -abs(mag) : abs(mag);
}

float SafeSqrt(float x) { return sqrt(max(0.0f, x)); }

float3 EaSquareToSphere(float2 uv)
{
	// Transform to [-1, 1]^2
    float2 a = 2 * uv - 1;
	float2 absa = abs(a);

	// Compute radius and angle
	float signedDist = 1 - (absa.x + absa.y); // Signed distance to the u + v = 1 diagonal diamond
	float d = abs(signedDist);
	float r = 1 - d;
	float phi = (r == 0 ? 1 : (absa.y - absa.x) / r + 1) * PI / 4;

	// Compute vector
	float y = CopySign(1 - r * r, signedDist);
	float cosPhi = CopySign(cos(phi), a.x);
	float sinPhi = CopySign(sin(phi), a.y);
	return float3(cosPhi * r * SafeSqrt(2 - r * r), y, sinPhi * r * SafeSqrt(2 - r * r));
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
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= gOutputWidth || dispatchThreadID.y >= gOutputHeight)
        return;

    // Normalized coordinates in [0,1]
    float u = (dispatchThreadID.x + 0.5f) / gOutputWidth;
    float v = (dispatchThreadID.y + 0.5f) / gOutputHeight;

    float3 dir = EaSquareToSphere(float2(u, v));
    float2 panoUV = PanoSphereToSquare(dir);
    panoUV.y = 1 - panoUV.y;

    float3 color = gPano.SampleLevel(gSampler, panoUV, 0.0f).rgb;

    gEA[dispatchThreadID.xy] = float4(color, 1);
}
