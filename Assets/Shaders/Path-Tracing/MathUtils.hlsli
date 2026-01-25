#ifndef H_MATH_UTILS_H
#define H_MATH_UTILS_H

// https://sakibsaikia.github.io/graphics/2022/01/04/Nan-Checks-In-HLSL.html
bool IsNaN(float x)
{
    return (asuint(x) & 0x7fffffff) > 0x7f800000;
}

bool IsNaN3(float3 x)
{
    return IsNaN(x.x) || IsNaN(x.y) || IsNaN(x.z);
}

#endif