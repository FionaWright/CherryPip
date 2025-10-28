#ifndef H_PTBUFFERS_H
#define H_PTBUFFERS_H

#include "HlslGlue.h"

struct PtInstanceData
{
    uint IndexBufferIdx;
    uint VertexBufferIdx;
    uint MaterialIdx;
    uint p;

    float4x4 M;
};

#endif