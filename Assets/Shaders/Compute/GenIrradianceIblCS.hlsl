#include "DualIncludes/HlslMath.h"

#define SAMPLE_DELTA 0.06f

TextureCube<float4> SrcCubemap : register(t0);
RWTexture2DArray<float4> DstIrradianceMap : register(u0);
SamplerState gSampler : register(s0);

[numthreads(8, 8, 6)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint face = DTid.z;
    uint2 uv = DTid.xy;

    int width, height, elements;
    DstIrradianceMap.GetDimensions(width, height, elements);
    float2 dimensions = float2(width, height);

    float2 uvt = (float2(uv) / float2(dimensions - 1)) * 2.0 - 1.0;
    float3 normal = CubemapCubeToSphere(face, uvt);
    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));

    float3 irradiance = 0;
    int samples = 0;
    for (float phi = 0; phi < 2 * PI; phi += SAMPLE_DELTA)
        for (float theta = 0; theta < PI / 2; theta += SAMPLE_DELTA)
        {
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

            float3 radiance = SrcCubemap.SampleLevel(gSampler, sampleVec, 0).rgb;
            irradiance += radiance * cos(theta) * sin(theta);
            samples++;
        }

    irradiance *= PI / float(samples);
    DstIrradianceMap[uint3(uv, face)] = float4(irradiance, 1.0);
}