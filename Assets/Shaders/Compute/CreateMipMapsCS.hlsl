#define BLOCK_SIZE 8

Texture2D<float4> SrcTexture : register(t0);
RWTexture2D<float4> DstTexture : register(u0);
SamplerState BilinearClamp : register(s0);

cbuffer CB : register(b0)
{
    float2 TexelSize; 
}

[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{   
    float2 texcoordsC = TexelSize * (DTid.xy);
    float2 texcoordsN = TexelSize * (DTid.xy + float2(0, 1));
    float2 texcoordsE = TexelSize * (DTid.xy + float2(1, 0));
    float2 texcoordsS = TexelSize * (DTid.xy + float2(0, -1));
    float2 texcoordsW = TexelSize * (DTid.xy + float2(-1, 0));

    float4 colorN = SrcTexture.Sample(BilinearClamp, texcoordsN);
    float4 colorE = SrcTexture.Sample(BilinearClamp, texcoordsE);
    float4 colorS = SrcTexture.Sample(BilinearClamp, texcoordsS);
    float4 colorW = SrcTexture.Sample(BilinearClamp, texcoordsW);
    float4 colorC = SrcTexture.Sample(BilinearClamp, texcoordsC);
    
    colorN.rgb = pow(colorN.rgb, 2.2);
    colorE.rgb = pow(colorE.rgb, 2.2);
    colorS.rgb = pow(colorS.rgb, 2.2);
    colorW.rgb = pow(colorW.rgb, 2.2);
    colorC.rgb = pow(colorC.rgb, 2.2);
    
    float4 averagedColor = (colorN + colorE + colorS + colorW + colorC) / 5.0f;
    averagedColor.rgb = pow(averagedColor.rgb, 1.0f / 2.2f);   

    DstTexture[DTid.xy] = averagedColor;
}