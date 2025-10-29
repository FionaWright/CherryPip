//
// Created by fiona on 06/10/2025.
//

#ifndef PT_MATERIAL_H
#define PT_MATERIAL_H
#include <DirectXMath.h>
#include <memory>
#include <vector>

#include "HWI/D12Resource.h"
#include "HWI/Texture.h"

class TLAS;
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
    UINT HeapIndex;

    // TODO: Get rid of material ownership
    std::shared_ptr<Texture> Texture = nullptr;
    std::shared_ptr<TLAS> TLAS = nullptr;
    std::shared_ptr<D12Resource> Buffer = nullptr;
};

struct UAV
{
    UINT HeapIndex;

    // TODO: Get rid of material ownership
    std::shared_ptr<Texture> Texture = nullptr;
};

struct MaterialData
{
    DirectX::XMFLOAT3 BaseColorFactor = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
    float EmmissiveStrength = 0.0f;
};

class Material
{
public:
    ~Material();
    void Init(const Heap* heap);
    const MaterialData* GetData() const { return &m_materialData; }
    void SetData(const MaterialData& data) { m_materialData = data; }

    void AddCBV(ID3D12Device* device, Heap* heap, size_t size);
    void AddSRV(ID3D12Device* device, Heap* heap, std::shared_ptr<Texture> tex);
    void AddBuffer(ID3D12Device* device, Heap* heap, std::shared_ptr<D12Resource> resource, UINT numElements,
                   size_t stride);
    void AddTLAS(ID3D12Device* device, Heap* heap, const std::shared_ptr<TLAS>& tlas);
    void AddUAV(ID3D12Device* device, Heap* heap, const std::shared_ptr<Texture>& tex);
    void AddUAV(ID3D12Device* device, Heap* heap, ID3D12Resource* resource, DXGI_FORMAT format);

    void TransitionSrvsToPS(ID3D12GraphicsCommandList* cmdList) const;

    void UpdateCBV(UINT regIdx, const void* data) const;
    void SetDescriptorTables(ID3D12GraphicsCommandList* cmdList, bool isCompute = false) const;

private:
    D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle = {};
    UINT m_descriptorIncSize = 0;

    std::vector<CBV> m_cbvs = {};
    std::vector<SRV> m_srvs = {};
    std::vector<UAV> m_uavs = {};

    MaterialData m_materialData;
};


#endif //PT_MATERIAL_H
