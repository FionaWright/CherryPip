// Interface for the MicrofacetModel
// All microfacet model "subclasses" must provide implementations for these functions

float RoughnessToAlpha(float roughness);

float3 Sample(float u1, float u2);

float D(float3 H);
float G1(float3 W);
float G2(float3 L, float3 V);

float PDF(float D, float3 H, float3 V);

