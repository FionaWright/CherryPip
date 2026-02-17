//
// Created by fiona on 17/02/2026.
//

#ifndef CHERRYPIP_PATHVISUALIZER_H
#define CHERRYPIP_PATHVISUALIZER_H
#include <cstdint>

#include "PTBuffers.h"
#include "ReadbackBuffer.h"

class PathVisualizer
{
public:
    void Init(D3D* d3d, uint32_t maxSPP);

    [[nodiscard]] const D12Resource* GetStructuredBuffer() const { return &m_structuredBuffer; }
    [[nodiscard]] uint32_t GetNumElements() const { return m_maxSPP; }
    std::vector<DebugPathVisualization> ReadbackData(D3D* d3d);

private:
    D12Resource m_structuredBuffer;
    D12Resource m_readbackBuffer;
    size_t m_bufferSize = 0;
    uint32_t m_maxSPP = 0;
};


#endif //CHERRYPIP_PATHVISUALIZER_H