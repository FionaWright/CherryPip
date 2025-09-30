//
// Created by fiona on 30/09/2025.
//

#ifndef PT_TEXTURE_H
#define PT_TEXTURE_H

#include <d3d12.h>
#include <string>

#include "D12Resource.h"

class Texture
{
public:
    void Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::wstring filePath, const DXGI_FORMAT format, const int arraySize, const D3D12_RESOURCE_FLAGS flags);

private:
    int m_width = -1, m_height = -1;
    D12Resource m_resource;
};


#endif //PT_TEXTURE_H