//
// Created by fiona on 30/09/2025.
//

#include "HWI/Texture.h"

#include <cmath>

#include "Helper.h"
#include "System/TextureLoader.h"

size_t BitsPerPixel(_In_ DXGI_FORMAT fmt)
{
    switch (fmt)
    {
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    case DXGI_FORMAT_R32G32B32A32_SINT:
        return 128;

    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32_UINT:
    case DXGI_FORMAT_R32G32B32_SINT:
        return 96;

    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_UINT:
    case DXGI_FORMAT_R16G16B16A16_SNORM:
    case DXGI_FORMAT_R16G16B16A16_SINT:
    case DXGI_FORMAT_R32G32_FLOAT:
    case DXGI_FORMAT_R32G32_UINT:
    case DXGI_FORMAT_R32G32_SINT:
        return 64;

    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
    case DXGI_FORMAT_R8G8B8A8_SINT:
        return 32;

    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_R8_UINT:
    case DXGI_FORMAT_R8_SNORM:
    case DXGI_FORMAT_R8_SINT:
        return 8;

    // TODO: Bch, Depth, etc

    default:
        return 0;
    }
}

void Texture::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::wstring filePath, const DXGI_FORMAT format, const int arraySize, const D3D12_RESOURCE_FLAGS flags)
{
    uint8_t* pData;
    bool hasAlpha = false, flip = false, isNormal = false;
    int channels = -1;
    std::string nwPath = wstringToString(filePath);
    TextureLoader::LoadTex(nwPath, m_width, m_height, &pData, hasAlpha, channels, flip, isNormal);

    const int maxDim = std::max<int>(m_width, m_height);

    D3D12_RESOURCE_DESC desc = {};
    desc.Width = m_width;
    desc.Height = m_height;
    desc.Format = format;
    desc.MipLevels = static_cast<UINT16>(std::log2(maxDim) + 1.0);
    desc.DepthOrArraySize = arraySize;
    desc.Flags = flags;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 1;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    m_resource.Init(device, desc, D3D12_RESOURCE_STATE_COPY_DEST);

    const int rowPitch = m_width * (BitsPerPixel(format) / 8);
    const int totalBytes = rowPitch * m_height;
    m_resource.Upload(cmdList, &pData, totalBytes, rowPitch);

    std::wstring debugName(filePath.begin(), filePath.end());
    m_resource.GetResource()->SetName(debugName.c_str());

    if (desc.MipLevels > 1)
    {
        m_resource.Transition(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        TextureLoader::CreateMipMaps(device, cmdList, &m_resource);
    }
}
