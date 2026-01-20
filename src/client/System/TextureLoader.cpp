//
// Created by fiona on 30/09/2025.
//

#include "System/pch.h"
#include "System/TextureLoader.h"
#include "../../../Headers/client/MathUtils.h"
#include "../../../Headers/client/Helper.h"
#include "HWI/D12Resource.h"

#include "spng.h"
#include "tinyddsloader.h"

using std::wstring;
using std::ofstream;

#pragma warning (disable : 6386)
#pragma warning (disable : 6385)

#define BLOCK_SIZE 8
constexpr int NUM_CHANNELS = 4;

ComPtr<ID3D12RootSignature> TextureLoader::ms_rootSigMipMap, TextureLoader::ms_rootSigMipMapCubemap;
Shader TextureLoader::ms_shaderMipMap, TextureLoader::ms_shaderMipMapCubemap;
std::vector<ComPtr<ID3D12DescriptorHeap>> TextureLoader::ms_trackedDescHeaps;

void TextureLoader::LoadTex(const std::string& filePath, int& width, int& height, uint8_t** pData, DXGI_FORMAT& format)
{
    const size_t dotIndex = filePath.find_last_of('.');
    if (dotIndex == std::string::npos)
        throw new std::exception("Invalid file path");

    const std::string fileExtension = filePath.substr(dotIndex + 1, filePath.size() - dotIndex - 1);

    if (fileExtension == "dds")
    {
        LoadDDS(filePath, width, height, pData, format);
        return;
    }

    if (fileExtension == "png")
    {
        LoadPNG(filePath, width, height, pData, format);
        return;
    }

    if (fileExtension == "hdr")
    {
        LoadHDR(filePath, width, height, pData, format);
        return;
    }

    throw new std::exception(("Invalid texture file type: ." + fileExtension).c_str());
}

void TextureLoader::LoadDDS(const std::string& filePath, int& width, int& height, uint8_t** pData, DXGI_FORMAT& format)
{
    tinyddsloader::DDSFile dds;
    const auto r = dds.Load(filePath.c_str());
    if(r != tinyddsloader::Result::Success)
        throw std::exception("Error loading DDS");

    width = dds.GetWidth();
    height = dds.GetHeight();
    uint32_t mips   = dds.GetMipCount();
    format = static_cast<DXGI_FORMAT>(dds.GetFormat());    // BC1/BC3/BC5/BC7 etc.

    // TODO: Load all mipmaps
    const tinyddsloader::DDSFile::ImageData* img = dds.GetImageData(0, 0);

    const size_t dataSize  = img->m_memSlicePitch;
    *pData = new uint8_t[dataSize];
    memcpy(*pData, img->m_mem, dataSize);         // compressed blocks
}

int GetChannelsFromColorType(int color_type)
{
    switch (color_type)
    {
    case SPNG_COLOR_TYPE_GRAYSCALE:
        return 1; // 1 channel: grayscale
    case SPNG_COLOR_TYPE_GRAYSCALE_ALPHA:
        return 2; // 2 channels: grayscale + alpha
    case SPNG_COLOR_TYPE_TRUECOLOR:
        return 3; // 3 channels: RGB
    case SPNG_COLOR_TYPE_INDEXED:
        return 3; // 3 channels: RGB (palette)
    case SPNG_COLOR_TYPE_TRUECOLOR_ALPHA:
        return 4; // 4 channels: RGBA
    default:
        throw std::runtime_error("Unknown color type");
    }
}

void TextureLoader::LoadPNG(const std::string& filePath, int& width, int& height, uint8_t** pData, DXGI_FORMAT& format)
{
    FILE* file;
    fopen_s(&file, filePath.c_str(), "rb");
    if (!file)
        throw std::exception("I/O Error");

    spng_ctx* ctx = spng_ctx_new(0);

    spng_set_png_file(ctx, file);

    spng_ihdr ihdr;
    spng_get_ihdr(ctx, &ihdr);

    width = ihdr.width;
    height = ihdr.height;
    uint32_t channels = GetChannelsFromColorType(ihdr.color_type);

    if (filePath == "Assets/Textures/Transparent.png")
        channels = 4;

    spng_format spngFormat =
        channels == 4 ? SPNG_FMT_RGBA8 :
        channels == 3 ? SPNG_FMT_RGBA8 : // No existing RGB8 DXGI format so we have to leave the alpha empty
        channels == 2 ? SPNG_FMT_GA8 :
        SPNG_FMT_G8;

    size_t outSize;
    spng_decoded_image_size(ctx, spngFormat, &outSize);
    *pData = new uint8_t[outSize];

    spng_decode_image(ctx, *pData, outSize, spngFormat, 0);

    format = DXGI_FORMAT_R8G8B8A8_UNORM;

    spng_ctx_free(ctx);
    fclose(file);
}

void TextureLoader::LoadPNG(const uint8_t* inputData, const uint32_t dataSize, int& width, int& height, uint8_t** pData, int& channels)
{
    assert(inputData && dataSize > 0);

    spng_ctx* ctx = spng_ctx_new(0);
    assert(ctx);

    spng_set_png_buffer(ctx, inputData, dataSize);

    spng_ihdr ihdr;
    spng_get_ihdr(ctx, &ihdr);

    width = ihdr.width;
    height = ihdr.height;
    channels = GetChannelsFromColorType(ihdr.color_type);

    spng_format format =
        channels == 4 ? SPNG_FMT_RGBA8 :
        channels == 3 ? SPNG_FMT_RGBA8 : // No existing RGB8 DXGI format so we have to leave the alpha empty
        channels == 2 ? SPNG_FMT_GA8 :
        SPNG_FMT_G8;

    size_t outSize;
    spng_decoded_image_size(ctx, format, &outSize);
    *pData = new uint8_t[outSize];

    spng_decode_image(ctx, *pData, outSize, format, 0);

    spng_ctx_free(ctx);
}

XMFLOAT3 rgbeToFloat(const uint8_t* rgbe)
{
    if (rgbe[3] == 0)
        return XMFLOAT3(0, 0, 0);

    float f = ldexp(1.0f, rgbe[3] - (int)(128 + 8));
    return XMFLOAT3(rgbe[0] * f, rgbe[1] * f, rgbe[2] * f);
}

XMFLOAT3 SampleHDR(const uint8_t* hdrData, const XMFLOAT3& direction, const int hdrWidth, const int hdrHeight)
{
    float theta = atan2f(direction.z, direction.x);
    float phi = acos(direction.y);

    float texU = ((theta / (2.0f * static_cast<float>(PI))) + 0.5f) * hdrWidth;
    float texV = (phi / static_cast<float>(PI)) * hdrHeight;

    texU = std::clamp(texU, 0.0f, static_cast<float>(hdrWidth - 1));
    texV = std::clamp(texV, 0.0f, static_cast<float>(hdrHeight - 1));

    int x = static_cast<int>(texU);
    int y = static_cast<int>(texV);
    int x1 = std::min<int>(x + 1, hdrWidth - 1);
    int y1 = std::min<int>(y + 1, hdrHeight - 1);
    float dx = texU - x;
    float dy = texV - y;

    const uint8_t* c00 = &hdrData[(y * hdrWidth + x) * 4];
    XMFLOAT3 col00 = rgbeToFloat(c00);

    const uint8_t* c10 = &hdrData[(y * hdrWidth + x1) * 4];
    XMFLOAT3 col10 = rgbeToFloat(c10);

    const uint8_t* c01 = &hdrData[(y1 * hdrWidth + x) * 4];
    XMFLOAT3 col01 = rgbeToFloat(c01);

    const uint8_t* c11 = &hdrData[(y1 * hdrWidth + x1) * 4];
    XMFLOAT3 col11 = rgbeToFloat(c11);

    col00 = Mult(col00, (1 - dx) * (1 - dy));
    col10 = Mult(col10, dx * (1 - dy));
    col01 = Mult(col01, dy * (1 - dx));
    col11 = Mult(col11, dx * dy);

    return Add(Add(col00, col10), Add(col01, col11));
}

void ReadRLEData(std::ifstream& fin, uint8_t*& data, int width, int height)
{
    if (width < 8 || width > 0x7fff)
    {
        fin.read(reinterpret_cast<char*>(data), width * height * 4);
        return;
    }

    uint8_t* scanline_buffer = new uint8_t[width * 4];

    for (int y = 0; y < height; y++)
    {
        uint8_t rgbe[4];
        fin.read(reinterpret_cast<char*>(rgbe), 4);

        if (rgbe[0] != 2 || rgbe[1] != 2 || (rgbe[2] & 0x80))
        {
            // This file is not run length encoded
            data[(y * width + 0) * 4 + 0] = rgbe[0];
            data[(y * width + 0) * 4 + 1] = rgbe[1];
            data[(y * width + 0) * 4 + 2] = rgbe[2];
            data[(y * width + 0) * 4 + 3] = rgbe[3];
            fin.read(reinterpret_cast<char*>(&data[(y * width + 1) * 4]), (width - 1) * 4);
            continue;
        }

        if (((rgbe[2] << 8) | rgbe[3]) != width)
            throw std::exception("Invalid scanline width");

        for (int i = 0; i < 4; i++)
        {
            int ptr = 0;
            while (ptr < width)
            {
                uint8_t buf[2];
                fin.read(reinterpret_cast<char*>(buf), 2);
                if (buf[0] > 128)
                {
                    int count = buf[0] - 128;
                    if ((count == 0) || (count > width - ptr))
                        throw std::exception("Bad RLE data");

                    while (count > 0)
                    {
                        scanline_buffer[ptr] = buf[1];
                        ptr++;
                        count--;
                    }
                    continue;
                }

                int count = buf[0];
                if ((count == 0) || (count > width - ptr))
                    throw std::exception("Bad RLE data");

                scanline_buffer[ptr++] = buf[1];
                count--;
                if (count > 0)
                {
                    fin.read(reinterpret_cast<char*>(&scanline_buffer[ptr]), count);
                    ptr += count;
                }
            }

            for (int j = 0; j < width; j++)
            {
                data[(y * width + j) * 4 + i] = scanline_buffer[j];
            }
        }
    }
    delete[] scanline_buffer;
}

void TextureLoader::LoadHDR(std::string filePath, int& width, int& height, uint8_t** pData, DXGI_FORMAT& format)
{
    size_t dotIndex = filePath.find_last_of('.');
    if (dotIndex == std::string::npos)
        throw new std::exception("Invalid file path");

    std::ifstream fin;
    fin.open(filePath, std::ios::binary);

    if (!fin)
        throw std::exception("IO Exception");

    std::string confirmation;
    std::getline(fin, confirmation);
    if (confirmation != "#?RADIANCE")
        throw std::exception("Invalid HDR file");

    std::string formatStr = "";
    while (formatStr.empty() ||
            formatStr.starts_with("# Made") ||
            formatStr.starts_with("GAMMA=") ||
            formatStr.starts_with("PRIMARIES="))
        std::getline(fin, formatStr);
    if (formatStr != "FORMAT=32-bit_rle_rgbe")
        throw std::exception("Invalid HDR file");

    fin.get();

    std::string sizeStr;
    std::getline(fin, sizeStr);
    size_t Yindex = sizeStr.find_first_of('Y');
    size_t Xindex = sizeStr.find_first_of('X');
    int hdrHeight = atoi(sizeStr.substr(Yindex + 2, Xindex - 2 - Yindex + 2).c_str());
    int hdrWidth = atoi(sizeStr.substr(Xindex + 2, sizeStr.size() - Xindex + 1).c_str());

    uint32_t channels = 4; // Rgbe8
    int totalPixelCount = hdrWidth * hdrHeight;
    int totalBytes = totalPixelCount * channels;

    uint8_t* hdrData = new uint8_t[totalBytes];
    ReadRLEData(fin, hdrData, hdrWidth, hdrHeight);

    height = hdrHeight;
    width = hdrWidth;

    format = DXGI_FORMAT_R32G32B32_FLOAT;
    *pData = new uint8_t[width * height * sizeof(float) * 3];
    float* outData = reinterpret_cast<float*>(*pData);

    for (int i = 0; i < width * height; ++i)
    {
        uint8_t r = hdrData[i * 4 + 0];
        uint8_t g = hdrData[i * 4 + 1];
        uint8_t b = hdrData[i * 4 + 2];
        uint8_t e = hdrData[i * 4 + 3];

        float f = (e) ? ldexp(1.0f, e - (128 + 8)) : 0.0f;
        outData[i * 3 + 0] = r * f;
        outData[i * 3 + 1] = g * f;
        outData[i * 3 + 2] = b * f;
    }

    fin.close();
    delete[] hdrData;
}

void TextureLoader::CreateMipMaps(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, D12Resource* resource)
{
    const auto desc = resource->GetDesc();

    if (desc.MipLevels <= 1 || desc.DepthOrArraySize != 1)
        return;

    if (!ms_rootSigMipMap)
        throw std::exception();

    D3D12_SHADER_RESOURCE_VIEW_DESC srcSRVDesc = {};
    srcSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srcSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srcSRVDesc.Format = desc.Format;

    D3D12_UNORDERED_ACCESS_VIEW_DESC dstUAVDesc = {};
    dstUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    dstUAVDesc.Format = desc.Format;

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = 2 * (desc.MipLevels - 1);
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    ComPtr<ID3D12DescriptorHeap> heapForCS;
    V(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heapForCS)));
    ms_trackedDescHeaps.push_back(heapForCS);

    ID3D12DescriptorHeap* heaps = heapForCS.Get();
    cmdList->SetComputeRootSignature(ms_rootSigMipMap.Get());
    cmdList->SetDescriptorHeaps(1, &heaps);
    cmdList->SetPipelineState(ms_shaderMipMap.GetPSO());

    auto cpuHandle = heapForCS->GetCPUDescriptorHandleForHeapStart();
    auto gpuHandle = heapForCS->GetGPUDescriptorHandleForHeapStart();

    int width = static_cast<int>(desc.Width);
    int height = static_cast<int>(desc.Height);

    ID3D12Resource* pResource = resource->GetResource();

    const uint32_t descIncSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    for (int mip = 0; mip < desc.MipLevels - 1; mip++)
    {
        int dstWidth = std::max<int>(width >> (mip + 1), 1);
        int dstHeight = std::max<int>(height >> (mip + 1), 1);

        {
            float texelWidth = 1.0f / static_cast<float>(dstWidth);
            float texelHeight = 1.0f / static_cast<float>(dstHeight);

            cmdList->SetComputeRoot32BitConstant(0, *reinterpret_cast<UINT*>(&texelWidth), 0);
            cmdList->SetComputeRoot32BitConstant(0, *reinterpret_cast<UINT*>(&texelHeight), 1);
            //if (SettingsManager::ms_Dynamic.MipMapDebugMode)
            //    cmdList->SetComputeRoot32BitConstant(0, mip, 2);
        }

        srcSRVDesc.Texture2D.MipLevels = 1;
        srcSRVDesc.Texture2D.MostDetailedMip = mip;

        dstUAVDesc.Texture2D.MipSlice = mip + 1;

        CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandleSrc(cpuHandle, mip * 2, descIncSize);
        CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandleSrc(gpuHandle, mip * 2, descIncSize);

        CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandleDst(cpuHandle, mip * 2 + 1, descIncSize);
        CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandleDst(gpuHandle, mip * 2 + 1, descIncSize);

        device->CreateShaderResourceView(pResource, &srcSRVDesc, cpuHandleSrc);
        device->CreateUnorderedAccessView(pResource, nullptr, &dstUAVDesc, cpuHandleDst);

        cmdList->SetComputeRootDescriptorTable(1, gpuHandleSrc);
        cmdList->SetComputeRootDescriptorTable(2, gpuHandleDst);

        cmdList->Dispatch(std::max<int>(dstWidth / 8, 1), std::max<int>(dstHeight / 8, 1), 1);

        auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(pResource);
        cmdList->ResourceBarrier(1, &uavBarrier);
    }
}

void TextureLoader::CreateMipMapsCubemap(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
    D12Resource* resource)
{
    const auto desc = resource->GetDesc();

    if (desc.MipLevels <= 1)
        return;

    if (!ms_rootSigMipMapCubemap)
        throw std::exception();

    D3D12_SHADER_RESOURCE_VIEW_DESC srcSRVDesc = {};
    srcSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srcSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srcSRVDesc.TextureCube.MipLevels = 1;
    srcSRVDesc.Format = desc.Format;

    D3D12_UNORDERED_ACCESS_VIEW_DESC dstUAVDesc = {};
    dstUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
    dstUAVDesc.Texture2DArray.ArraySize = 6;
    dstUAVDesc.Texture2DArray.FirstArraySlice = 0;
    dstUAVDesc.Format = desc.Format;

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = 2 * (desc.MipLevels - 1) * 6;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    ComPtr<ID3D12DescriptorHeap> heapForCS;
    V(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heapForCS)));
    ms_trackedDescHeaps.push_back(heapForCS);

    ID3D12DescriptorHeap* heaps = heapForCS.Get();
    cmdList->SetComputeRootSignature(ms_rootSigMipMapCubemap.Get());
    cmdList->SetDescriptorHeaps(1, &heaps);
    cmdList->SetPipelineState(ms_shaderMipMapCubemap.GetPSO());

    auto cpuHandle = heapForCS->GetCPUDescriptorHandleForHeapStart();
    auto gpuHandle = heapForCS->GetGPUDescriptorHandleForHeapStart();

    int width = static_cast<int>(desc.Width);
    int height = static_cast<int>(desc.Height);

    ID3D12Resource* pResource = resource->GetResource();
    const uint32_t descIncSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    struct CBV
    {
        uint32_t FaceReso[2];
        float Roughness;
        float p;
    } cbv;

    for (int mip = 0; mip < desc.MipLevels - 1; mip++)
    {
        int dstWidth = std::max<int>(width >> (mip + 1), 1);
        int dstHeight = std::max<int>(height >> (mip + 1), 1);

        cbv.FaceReso[0] = static_cast<uint32_t>(dstWidth);
        cbv.FaceReso[1] = static_cast<uint32_t>(dstHeight);
        cbv.Roughness = static_cast<float>(mip) / static_cast<float>(desc.MipLevels - 2);
        cmdList->SetComputeRoot32BitConstants(0, sizeof(cbv) / 4, &cbv, 0);

        srcSRVDesc.TextureCube.MostDetailedMip = mip;
        dstUAVDesc.Texture2DArray.MipSlice = mip + 1;

        CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandleSrc(cpuHandle, mip * 2, descIncSize);
        CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandleSrc(gpuHandle, mip * 2, descIncSize);

        CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandleDst(cpuHandle, mip * 2 + 1, descIncSize);
        CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandleDst(gpuHandle, mip * 2 + 1, descIncSize);

        device->CreateShaderResourceView(pResource, &srcSRVDesc, cpuHandleSrc);
        device->CreateUnorderedAccessView(pResource, nullptr, &dstUAVDesc, cpuHandleDst);

        cmdList->SetComputeRootDescriptorTable(1, gpuHandleSrc);
        cmdList->SetComputeRootDescriptorTable(2, gpuHandleDst);

        cmdList->Dispatch(std::max<int>(dstWidth / 8, 1), std::max<int>(dstHeight / 8, 1), 1);

        auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(pResource);
        cmdList->ResourceBarrier(1, &uavBarrier);
    }
}

void TextureLoader::Init(const D3D* d3d, const std::wstring& shadersPath)
{
    ID3D12Device* device = d3d->GetDevice();

    D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    samplerDesc.MaxAnisotropy = 0;
    samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
    samplerDesc.ShaderRegister = 0;
    samplerDesc.RegisterSpace = 0;
    samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    ID3DBlob* signature;
    ID3DBlob* error;

    {
        // The compute shader expects 2 floats, the source texture and the destination texture
        CD3DX12_DESCRIPTOR_RANGE srvCbvRanges[2];
        CD3DX12_ROOT_PARAMETER rootParameters[3];
        srvCbvRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
        srvCbvRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);

        constexpr int constantsCount = 2;
        rootParameters[0].InitAsConstants(constantsCount, 0);
        rootParameters[1].InitAsDescriptorTable(1, &srvCbvRanges[0]);
        rootParameters[2].InitAsDescriptorTable(1, &srvCbvRanges[1]);

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        V(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));

        V(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&ms_rootSigMipMap)));
    }

    {
        CD3DX12_DESCRIPTOR_RANGE srvCbvRanges[2];
        CD3DX12_ROOT_PARAMETER rootParameters[3];
        srvCbvRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
        srvCbvRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);

        constexpr int constantsCount = 4;
        rootParameters[0].InitAsConstants(constantsCount, 0);
        rootParameters[1].InitAsDescriptorTable(1, &srvCbvRanges[0]);
        rootParameters[2].InitAsDescriptorTable(1, &srvCbvRanges[1]);

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        V(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));

        V(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&ms_rootSigMipMapCubemap)));
    }

    ms_shaderMipMap.InitCs(L"Compute/CreateMipMapsCS.hlsl", device, ms_rootSigMipMap.Get());
    ms_shaderMipMapCubemap.InitCs(L"Compute/CreateCubemapMipMapCS.hlsl", device, ms_rootSigMipMapCubemap.Get());
}

bool TextureLoader::manuallyDetermineHasAlpha(size_t bytes, int channels, uint8_t* pData)
{
    if (channels != 4)
        return false;

    for (size_t i = channels - 1; i < bytes; i += channels)
    {
        const uint8_t a = pData[i];
        if (a < UINT8_MAX)
            return true;
    }
    return false;
}