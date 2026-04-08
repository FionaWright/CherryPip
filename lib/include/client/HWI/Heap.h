//
// Created by fiona on 06/10/2025.
//

#ifndef PT_HEAP_H
#define PT_HEAP_H

class Texture;

using Microsoft::WRL::ComPtr;

class Heap
{
public:
    void Init(const char* name, ID3D12Device* device, size_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type);
    UINT GetNextDescriptor(const char* debugName = nullptr);

    UINT GetNextDescriptorBindlessTexture(const char* debugName = nullptr);
    UINT AddBindlessTexture(ID3D12Device* device, std::shared_ptr<Texture> tex);
    [[nodiscard]] UINT GetBindlessTexBase() const { return m_baseBindlessTex; }

    void InitCBV(ID3D12Device* device, ID3D12Resource* resource, size_t size, UINT idx) const;
    void InitSRV(ID3D12Device* device, ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, UINT idx) const;
    void InitUAV(ID3D12Device* device, ID3D12Resource* resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc,
                 UINT idx) const;

    void SetHeap(ID3D12GraphicsCommandList* cmdList) const;
    void PrintHeapInfo() const;

    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return m_heapResource->GetCPUDescriptorHandleForHeapStart(); }
    [[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return m_heapResource->GetGPUDescriptorHandleForHeapStart(); }
    [[nodiscard]] UINT GetIncrementSize() const { return m_descriptorIncSize; }
    [[nodiscard]] D3D12_DESCRIPTOR_HEAP_TYPE GetType() const { return m_type; }
    [[nodiscard]] const std::vector<const char*>& GetDebugDescriptorList() const { return m_debugDescriptorNames; }
    [[nodiscard]] const std::vector<const char*>& GetDebugDescriptorListBindless() const { return m_debugDescriptorNamesBindless; }
    [[nodiscard]] uint32_t GetCurrBindedDescriptorCount() const { return m_currentHeapIndex; }
    [[nodiscard]] uint32_t GetCurrBindlessDescriptorCount() const { return m_currentHeapIndexBindlessTex - m_baseBindlessTex; }

private:
    std::string m_name;
    ComPtr<ID3D12DescriptorHeap> m_heapResource;
    UINT m_descriptorIncSize = 0;
    D3D12_DESCRIPTOR_HEAP_TYPE m_type = {};

    // Used only when flag --debugHeap given
    std::vector<const char*> m_debugDescriptorNames = {};
    std::vector<const char*> m_debugDescriptorNamesBindless = {};

    size_t m_baseBindlessTex = 0;
    size_t m_currentHeapIndex = 0;
    size_t m_currentHeapIndexBindlessTex = 0;
    size_t m_heapSize = 0;
};


#endif //PT_HEAP_H