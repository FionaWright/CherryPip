//
// Created by fionaw on 03/11/2025.
//

#ifndef CHERRYPIP_READBACKBUFFER_H
#define CHERRYPIP_READBACKBUFFER_H
#include <vector>

#include "HWI/D3D.h"
#include "HWI/Texture.h"


class ReadbackBuffer
{
public:
    void Init(const D3D* d3d, ID3D12GraphicsCommandList* cmdList, Texture* texture);
    void Readback(D3D* d3d, ID3D12GraphicsCommandList* cmdList);

    const uint8_t* GetData() const { return m_readbackData.data(); }

private:
    ComPtr<ID3D12Resource> m_readbackBuffer;
    Texture* m_assignedTex = nullptr;
    size_t m_bufferSize = 0;

    std::vector<uint8_t> m_readbackData;
};


#endif //CHERRYPIP_READBACKBUFFER_H