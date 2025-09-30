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

    void Transition(ID3D12GraphicsCommandList* cmdList, D3D12_RESOURCE_STATES newState, UINT subresourceIdx = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) { m_resource.Transition(cmdList, newState, subresourceIdx); }

    ID3D12Resource* GetResource() const { return m_resource.GetResource(); }
    DXGI_FORMAT GetFormat() const { return m_resource.GetDesc().Format; }
    D3D12_RESOURCE_DESC GetDesc() const { return m_resource.GetDesc(); }

private:
    int m_width = -1, m_height = -1;
    D12Resource m_resource;
};


#endif //PT_TEXTURE_H