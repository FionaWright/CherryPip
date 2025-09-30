//
// Created by fiona on 30/09/2025.
//

#ifndef PT_D12RESOURCE_H
#define PT_D12RESOURCE_H
#include <cstdint>
#include <d3d12.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class D12Resource
{
public:
    void Init(ID3D12Device* device, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_RESOURCE_STATES& initialState);
    void Upload(ID3D12GraphicsCommandList* cmdList, uint8_t** pData, size_t totalBytes, size_t rowPitch) const;

    void Transition(ID3D12GraphicsCommandList* cmdList, const D3D12_RESOURCE_STATES& newState, UINT subresourceIdx = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

    ID3D12Resource* GetResource() const { return m_resource.Get(); };
    D3D12_RESOURCE_STATES GetCurrentState() const { return m_currentState; };
    D3D12_RESOURCE_DESC GetDesc() const { return m_desc; };

private:
    ComPtr<ID3D12Resource> m_uploadHeap;
    ComPtr<ID3D12Resource> m_resource;
    D3D12_RESOURCE_DESC m_desc = {};
    D3D12_RESOURCE_STATES m_currentState = {};
};

#endif //PT_D12RESOURCE_H
