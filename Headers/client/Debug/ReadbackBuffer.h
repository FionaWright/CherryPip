//
// Created by fionaw on 03/11/2025.
//

#ifndef CHERRYPIP_READBACKBUFFER_H
#define CHERRYPIP_READBACKBUFFER_H
#include <functional>
#include <vector>

#include "HWI/D3D.h"
#include "HWI/Texture.h"


class ReadbackBuffer
{
public:
    void Init(const D3D* d3d, ID3D12GraphicsCommandList* cmdList, Texture* texture);
    void Readback(D3D* d3d);
    void ReadbackAndAlter(D3D* d3d, const std::function<void(std::vector<uint8_t>&)>& alterData);

    const uint8_t* GetData() const { return m_readbackData.data(); }

private:
    void copyToBuffer(D3D* d3d) const;
    void copyToTexture(D3D* d3d) const;

    D12Resource m_readbackBuffer;
    Texture* m_assignedTex = nullptr;
    size_t m_bufferSize = 0;

    std::vector<uint8_t> m_readbackData;
};


#endif //CHERRYPIP_READBACKBUFFER_H