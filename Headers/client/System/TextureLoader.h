//
// Created by fiona on 30/09/2025.
//

#ifndef PT_TEXTURELOADER_H
#define PT_TEXTURELOADER_H
#include <cstdint>
#include <d3d12.h>
#include <string>
#include <vector>

#include <wrl/client.h>

#include "HWI/Shader.h"
using Microsoft::WRL::ComPtr;

class D12Resource;

class TextureLoader
{
    public:
	static void LoadTex(const std::string& filePath, int& width, int& height, uint8_t** pData, bool& hasAlpha, int& channels, bool flipUpsideDown = false, bool isNormalMap = false);

	static void LoadTGA(std::string filePath, int& width, int& height, uint8_t** pData);
	static void LoadDDS(const std::string& filePath, int& width, int& height, uint8_t** pData, bool& hasAlpha, int& channels);
	static void LoadDDS_DXT1(std::ifstream& fin, int& width, int& height, uint8_t** pData);
	static void LoadDDS_DXT5(std::ifstream& fin, int& width, int& height, uint8_t** pData);
	static void LoadDDS_ATI2(std::ifstream& fin, int& width, int& height, uint8_t** pData);
	static void LoadPNG(const std::string& filePath, int& width, int& height, uint8_t** pData, int& channels);
    static void LoadPNG(const uint8_t* inputData, uint32_t dataSize, int& width, int& height, uint8_t** pData, int& channels);
    static void LoadHDR(std::string filePath, int& width, int& height, std::vector<uint8_t*>& pDatas, int& channels);

	static void CreateMipMaps(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, D12Resource* resource);

	static void Init(D3D* d3d, const std::wstring& shadersPath);

private:
	static bool manuallyDetermineHasAlpha(size_t bytes, int channels, uint8_t* pData);

	static ComPtr<ID3D12RootSignature> ms_mipMapRootSig;
	static Shader ms_mipMapShader;

	static std::vector<ComPtr<ID3D12DescriptorHeap>> ms_trackedDescHeaps;
};


#endif //PT_TEXTURELOADER_H