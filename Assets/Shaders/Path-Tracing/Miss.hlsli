#ifndef H_MISSPS_H
#define H_MISSPS_H

float3 Miss(float3 origin, float3 direction)
{
#if defined(FURNACE_TEST_EMISSIVE)
    return float3(0, 0, 0);
#elif defined(FURNACE_TEST_CLASSIC)
    return float3(1, 1, 1);
#else
    return float3(0, 0, 0); // TODO
#endif
}

#endif