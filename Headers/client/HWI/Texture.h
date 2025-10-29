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
    ~Texture();
    void Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::string filePath, DXGI_FORMAT format,
              int arraySize = 1, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    void Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::wstring filePath, DXGI_FORMAT format,
              int arraySize = 1, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
    void InitEmpty(ID3D12Device* device, DXGI_FORMAT format, UINT width, UINT height, int arraySize = 1,
                   D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    void InitPNG(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const uint8_t* inData,
                 size_t dataSize,
                 DXGI_FORMAT format, int arraySize, D3D12_RESOURCE_FLAGS flags);

    void Transition(ID3D12GraphicsCommandList* cmdList, D3D12_RESOURCE_STATES newState,
                    UINT subresourceIdx = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
    {
        m_resource.Transition(cmdList, newState, subresourceIdx);
    }

    D12Resource* GetD12Resource() { return &m_resource; }
    DXGI_FORMAT GetFormat() const { return m_resource.GetDesc().Format; }
    D3D12_RESOURCE_DESC GetDesc() const { return m_resource.GetDesc(); }

private:
    int m_width = -1, m_height = -1;
    D12Resource m_resource;
};


#endif //PT_TEXTURE_H
