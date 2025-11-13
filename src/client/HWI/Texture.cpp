//
// Created by fiona on 30/09/2025.
//

#include "HWI/Texture.h"

#include <cmath>

#include "Helper.h"
#include "Debug/Profiler.h"
#include "System/TextureLoader.h"

size_t BitsPerPixel(_In_ const DXGI_FORMAT fmt, bool& isBC)
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

    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
        isBC = true; return 8;
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
        isBC = true; return 16;
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
        isBC = true; return 16;

    default:
        throw std::exception("Unsupported DXGI format");
    }
}

Texture::~Texture()
{
}

void Texture::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::string filePath,
                   const int arraySize, const D3D12_RESOURCE_FLAGS flags)
{
    const std::wstring wstr = std::wstring(filePath.begin(), filePath.end());
    Init(device, cmdList, wstr, arraySize, flags);
}

void Texture::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const std::wstring& filePath,
                   const int arraySize, const D3D12_RESOURCE_FLAGS flags)
{
    Profiler::AddToStack(filePath);

    uint8_t* pData = nullptr;
    DXGI_FORMAT format = {};
    const std::string nwPath = wstringToString(filePath);
    TextureLoader::LoadTex(nwPath, m_width, m_height, &pData, format);

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

    m_resource.Init(filePath.c_str(), device, desc, D3D12_RESOURCE_STATE_COPY_DEST);

    bool isBC = false;
    const uint32_t bpp = BitsPerPixel(format, isBC); // Note: For BC it's not really "Bits Per Pixel" but Block Size

    if (isBC)
    {
        const size_t rowPitch = ((m_width + 3) / 4) * bpp;
        const size_t slicePitch = rowPitch * ((m_height + 3) / 4);
        m_resource.UploadTexture(device, cmdList, pData, slicePitch, rowPitch);
    }
    else
    {
        const size_t rowPitch = m_width * (bpp / 8);
        const size_t totalBytes = rowPitch * m_height;
        m_resource.UploadTexture(device, cmdList, pData, totalBytes, rowPitch);
    }
    delete pData;

    if (desc.MipLevels > 1 && false)
    {
        m_resource.Transition(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        TextureLoader::CreateMipMaps(device, cmdList, &m_resource);
    }

    Profiler::PopAndPrint();
}

void Texture::InitEmpty(ID3D12Device* device, const DXGI_FORMAT format, const UINT width, const UINT height, const int arraySize,
                        const D3D12_RESOURCE_FLAGS flags)
{
    m_width = width;
    m_height = height;

    D3D12_RESOURCE_DESC desc = {};
    desc.Width = m_width;
    desc.Height = m_height;
    desc.Format = format;
    desc.MipLevels = 1;
    desc.DepthOrArraySize = arraySize;
    desc.Flags = flags;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    m_resource.Init(L"Empty Texture", device, desc, D3D12_RESOURCE_STATE_COPY_DEST);
}

void Texture::InitPNG(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const uint8_t* inData,
                      const size_t dataSize, const DXGI_FORMAT format, const int arraySize,
                      const D3D12_RESOURCE_FLAGS flags)
{
    uint8_t* pData = nullptr;
    int channels = -1;
    TextureLoader::LoadPNG(inData, dataSize, m_width, m_height, &pData, channels);

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

    m_resource.Init(L"PNG Texture", device, desc, D3D12_RESOURCE_STATE_COPY_DEST);

    bool isBC = false;
    const int rowPitch = m_width * (BitsPerPixel(format, isBC) / 8);
    const int totalBytes = rowPitch * m_height;
    m_resource.UploadTexture(device, cmdList, pData, totalBytes, rowPitch);
    delete pData;

    if (desc.MipLevels > 1)
    {
        m_resource.Transition(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        TextureLoader::CreateMipMaps(device, cmdList, &m_resource);
    }
}
