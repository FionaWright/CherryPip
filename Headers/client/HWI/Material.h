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
    D12Resource* Resource = nullptr;
};

struct UAV
{
    UINT HeapIndex;
};

struct MaterialData
{
    DirectX::XMFLOAT3 BaseColorFactor = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
    float EmissiveStrength = 0.0f;
};

class Material
{
public:
    ~Material();
    void Init(const Heap* heap);
    const MaterialData* GetData() const { return &m_materialData; }
    void SetData(const MaterialData& data) { m_materialData = data; }

    void AddCBV(ID3D12Device* device, Heap* heap, size_t size);
    void SetSRV(ID3D12Device* device, UINT srvIdx, Heap* heap, D12Resource* d12Resource,
                const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
    void SetTex(ID3D12Device* device, UINT srvIdx, Heap* heap, std::shared_ptr<Texture> tex);
    void SetBuffer(ID3D12Device* device, UINT srvIdx, Heap* heap, std::shared_ptr<D12Resource> resource,
                   UINT numElements,
                   size_t stride);
    void SetTlas(ID3D12Device* device, UINT srvIdx, Heap* heap, const std::shared_ptr<TLAS>& tlas);
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

    // TODO: Refactor this out!!!!
    std::vector<std::shared_ptr<Texture>> m_tempTextureOwnership;
    std::vector<std::shared_ptr<D12Resource>> m_tempResourceOwnership;
    std::vector<std::shared_ptr<TLAS>> m_tempTlasOwnership;
};


#endif //PT_MATERIAL_H
