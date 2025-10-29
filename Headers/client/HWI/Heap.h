//
// Created by fiona on 06/10/2025.
//

#ifndef PT_HEAP_H
#define PT_HEAP_H
#include <d3d12.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class Heap
{
public:
    void Init(ID3D12Device* device, size_t numDescriptors);
    UINT GetNextDescriptor();

    void InitCBV(ID3D12Device* device, ID3D12Resource* resource, size_t size, UINT idx) const;
    void InitSRV(ID3D12Device* device, ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, UINT idx) const;
    void InitUAV(ID3D12Device* device, ID3D12Resource* resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc,
                 UINT idx) const;

    void SetHeap(ID3D12GraphicsCommandList* cmdList) const;

    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return m_cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart(); }
    [[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return m_cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart(); }
    [[nodiscard]] UINT GetIncrementSize() const { return m_descriptorIncSize; }

private:
    ComPtr<ID3D12DescriptorHeap> m_cbvSrvUavHeap;
    UINT m_descriptorIncSize = 0;

    size_t m_currentHeapIndex = 0;
    size_t m_heapSize = 0;
};


#endif //PT_HEAP_H