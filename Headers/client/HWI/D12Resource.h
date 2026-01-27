//
// Created by fiona on 30/09/2025.
//

#ifndef PT_D12RESOURCE_H
#define PT_D12RESOURCE_H

class D12Resource
{
public:
    ~D12Resource();
    void InitBuffer(LPCWSTR name, ID3D12Device* device, size_t size, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE, bool readbackHeap = false);
    void InitRTAS(LPCWSTR name, ID3D12Device* device, size_t size, D3D12_RESOURCE_FLAGS flags);
    void Init(LPCWSTR name, ID3D12Device* device, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_RESOURCE_STATES& initialState, const D3D12_CLEAR_VALUE* clearValue = nullptr);
    void Fill(const ComPtr<ID3D12Resource>& resource, const D3D12_RESOURCE_STATES& initialState);

    void CreateHeap(ID3D12Device* device);
    void UploadBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const void* pData, size_t totalBytes);
    void UploadTexture(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const uint8_t* pData, size_t totalBytes, size_t rowPitch);
    void UploadTexture(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const uint8_t** pData, size_t totalBytes, size_t rowPitch);

    void Transition(ID3D12GraphicsCommandList* cmdList, const D3D12_RESOURCE_STATES& newState, UINT subresourceIdx = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
    void CopyTextureInto(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* srcResource, uint32_t dstX = 0, uint32_t dstY = 0,
                         uint32_t dstZ = 0, const D3D12_BOX* srcBox = nullptr) const;

    ID3D12Resource* GetResource() const { return m_resource.Get(); }
    ID3D12Resource* GetUploadResource() const { return m_uploadResource.Get(); }
    D3D12_RESOURCE_STATES GetCurrentState() const { return m_currentState; }
    D3D12_RESOURCE_DESC GetDesc() const { return m_desc; }
    [[nodiscard]] const std::wstring& GetDebugName() const { return m_debugName; }

private:
    ComPtr<ID3D12Resource> m_uploadResource;
    ComPtr<ID3D12Resource> m_resource;
    D3D12_RESOURCE_DESC m_desc = {};
    D3D12_RESOURCE_STATES m_currentState = {};

    std::wstring m_debugName;
};

#endif //PT_D12RESOURCE_H
