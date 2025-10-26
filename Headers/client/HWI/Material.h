//
// Created by fiona on 06/10/2025.
//

#ifndef PT_MATERIAL_H
#define PT_MATERIAL_H
#include <memory>
#include <vector>

#include "HWI/D12Resource.h"
#include "HWI/Texture.h"

class Heap;

struct CBV
{
    ComPtr<ID3D12Resource> Resource;
    const UINT HeapIndex;
    const size_t Size;
    const size_t AlignedSize;

    void* MappedGpuPtr;
};

struct SRV
{
    std::shared_ptr<Texture> Texture;
    UINT HeapIndex;
};

class Material
{
public:
    ~Material();
    void Init(const Heap* heap);
    void AddCBV(ID3D12Device* device, Heap* heap, size_t size);
    void AddSRV(ID3D12Device* device, Heap* heap, std::shared_ptr<Texture> tex);

    void TransitionSrvsToPS(ID3D12GraphicsCommandList* cmdList) const;

    void UpdateCBV(UINT regIdx, const void* data) const;
    void SetDescriptorTables(ID3D12GraphicsCommandList* cmdList) const;

private:
    D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle = {};
    UINT m_descriptorIncSize = 0;

    std::vector<CBV> m_cbvs;
    std::vector<SRV> m_srvs;
};


#endif //PT_MATERIAL_H