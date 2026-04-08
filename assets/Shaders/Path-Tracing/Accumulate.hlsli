#ifndef H_ACCUMULATE_H
#define H_ACCUMULATE_H

float3 AccumulateAndFetch(uint2 pixelCoord, float3 color, bool nanTestEnabled)
{
    if (!cbvPathTracing.AccumulationEnabled)
        return color;

    float3 accumColor = gAccum.Load(pixelCoord).rgb;
    if (IsNaN3(accumColor) && nanTestEnabled)
        return float3(1, 0, 1);

    float3 newSum = accumColor + color;

    float accumFrameCount = (float)cbvPathTracing.FrameIdx;
    float totalFrames = accumFrameCount + 1.0f;

    float3 average = (accumColor * accumFrameCount + color) / totalFrames;

    if (cbvPathTracing.UpdateAccumulation)
        gAccum[pixelCoord].rgb = average;

    return average;
}

#endif