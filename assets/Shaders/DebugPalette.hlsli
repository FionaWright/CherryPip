#ifndef H_DEBUGPALETTE_H
#define H_DEBUGPALETTE_H

float4 Palette(uint idx)
{
    const float3 colors[20] = {
        float3(1.0, 0.0, 0.0),   // Red
        float3(0.0, 1.0, 0.0),   // Green
        float3(0.0, 0.0, 1.0),   // Blue
        float3(1.0, 1.0, 0.0),   // Yellow
        float3(1.0, 0.0, 1.0),   // Magenta
        float3(0.0, 1.0, 1.0),   // Cyan
        float3(1.0, 0.5, 0.0),   // Orange
        float3(0.5, 0.0, 1.0),   // Purple
        float3(0.0, 0.5, 1.0),   // Sky blue
        float3(0.5, 1.0, 0.0),   // Lime
        float3(1.0, 0.0, 0.5),   // Pink-red
        float3(0.0, 1.0, 0.5),   // Aqua
        float3(0.5, 0.5, 0.5),   // Gray
        float3(1.0, 0.75, 0.0),  // Amber
        float3(0.75, 0.25, 0.0), // Brownish
        float3(0.25, 0.75, 1.0), // Light blue
        float3(0.75, 0.0, 0.75), // Violet
        float3(0.25, 1.0, 0.25), // Light green
        float3(1.0, 0.25, 0.25), // Light red
        float3(0.25, 0.25, 1.0)  // Light blue
    };

    return float4(colors[idx % 20], 1.0);
}

#endif
