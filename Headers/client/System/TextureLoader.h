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
	static void LoadPNG(std::string filePath, int& width, int& height, uint8_t** pData, int& channels);
	static void LoadHDR(std::string filePath, int& width, int& height, std::vector<uint8_t*>& pDatas, int& channels);

	static void CreateMipMaps(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, D12Resource* resource);

	static void Init(ID3D12Device* device, const std::wstring& assetsPath);

private:
	static bool manuallyDetermineHasAlpha(size_t bytes, int channels, uint8_t* pData);

	static ComPtr<ID3D12RootSignature> ms_mipMapRootSig;
	static ComPtr<ID3D12PipelineState> ms_mipMapPSO;

	static std::vector<ComPtr<ID3D12DescriptorHeap>> ms_trackedDescHeaps;
};


#endif //PT_TEXTURELOADER_H