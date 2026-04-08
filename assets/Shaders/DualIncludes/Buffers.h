#ifndef H_BUFFERS_H
#define H_BUFFERS_H

#include "HlslGlue.h"

struct MaxLumRedSearchStruct
{
    float Luminance;
    float2 UV;
};

struct SumSquaredErrorStruct
{
    float SquaredError;
};

#endif