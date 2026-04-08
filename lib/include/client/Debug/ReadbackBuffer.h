//
// Created by fionaw on 03/11/2025.
//

#ifndef CHERRYPIP_READBACKBUFFER_H
#define CHERRYPIP_READBACKBUFFER_H
#include "HWI/D3D.h"
#include "HWI/Texture.h"


class ReadbackBuffer
{
public:
    void Init(const D3D* d3d, const D12Resource* resource);
    void Readback(D3D* d3d, D12Resource* resource);

    [[nodiscard]] bool IsInitialized() const { return m_isInitialized; }

    std::vector<uint8_t>& GetData() { return m_readbackData; }

private:
    void copyToBuffer(D3D* d3d, D12Resource* resource) const;

    D12Resource m_readbackBuffer;
    size_t m_bufferSize = 0;

    bool m_isInitialized = false;

    size_t m_width = 0, m_height = 0;
    size_t m_rowPitch = 0;

    std::vector<uint8_t> m_readbackData;
};


#endif //CHERRYPIP_READBACKBUFFER_H