//
// Created by fionaw on 05/11/2025.
//

#ifndef CHERRYPIP_TEXTURERTV_H
#define CHERRYPIP_TEXTURERTV_H
#include <cstdint>
#include <d3d12.h>

#include "HWI/D12Resource.h"
#include "HWI/Heap.h"

class TextureRTV
{
public:
    void Init(LPCWSTR name, ID3D12Device* device, Heap* heap, uint32_t width, uint32_t height, DXGI_FORMAT format);

    [[nodiscard]] D12Resource* GetD12Resource() { return &m_d12Resource; }
    [[nodiscard]] ID3D12Resource* GetResource() const { return m_d12Resource.GetResource(); }
    [[nodiscard]] UINT GetHeapIdx() const { return m_heapIdx; }

private:
    D12Resource m_d12Resource;
    UINT m_heapIdx = 0;
};


#endif //CHERRYPIP_TEXTURERTV_H