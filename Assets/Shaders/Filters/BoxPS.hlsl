struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

ConstantBuffer<CbvFilterBox> c_box : register(b0);

Texture2D    gTex  : register(t0);
SamplerState gSampler : register(s0);

float4 PSMain(VsOut input) : SV_Target
{
    float4 sum = 0.0;
    int kernelSize = (c_box.Radius * 2 + 1);
    int sampleCount = kernelSize * kernelSize;

    for (int y = -c_box.Radius; y <= c_box.Radius; ++y)
    {
        for (int x = -c_box.Radius; x <= c_box.Radius; ++x)
        {
            float2 offset = float2(x, y) * c_box.TexelSize;
            sum += gTex.Sample(gSampler, input.uv + offset);
        }
    }

    return sum / sampleCount;
}