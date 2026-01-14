struct VsOut
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
};

float4 PSMain(VsOut input) : SV_TARGET
{
	return float4(input.normal, 1);
}