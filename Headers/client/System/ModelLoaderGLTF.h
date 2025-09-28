#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <string>
#include <iostream>
#include <fstream>
#include <intsafe.h>
#include <unordered_map>

#include "fastgltf/core.hpp"
#include "fastgltf/types.hpp"
#include "Render/Transform.h"

class Shader;
class Model;
class D3D;
using std::wstring;
using std::string;
using std::ifstream;
using std::ofstream;
using std::vector;
using std::unordered_map;

using Asset = std::shared_ptr<fastgltf::Expected<fastgltf::Asset>>;
using namespace DirectX;

#pragma pack(push, 1)
struct VertexInputDataGLTF
{
    XMFLOAT3 Position;
	XMFLOAT2 Texture;
	XMFLOAT3 Normal;
	XMFLOAT3 Tangent;
	XMFLOAT3 Binormal;
};
#pragma pack(pop)

struct GLTFLoadOverride
{
	UINT BatchIndex = -1;
	UINT ShaderIndex = -1;
	bool UseGlassSRVs = false;
	vector<string> WhiteList;
};

struct GLTFLoadArgs
{
	Transform Transform = {};

	vector<std::shared_ptr<Shader>> Shaders;

	UINT DefaultShaderIndex = 0;
	UINT DefaultShaderATIndex = 1;

	vector<string> CullingWhiteList;
	vector<GLTFLoadOverride> Overrides;
};

class ModelLoaderGLTF
{
public:
    static vector<std::shared_ptr<Model>> LoadModelsFromGLTF(D3D* d3d, ID3D12GraphicsCommandList2* cmdList, string modelName);

private:
	static Transform toTransform(fastgltf::TRS& trs);
	static void loadGLTFIndices(vector<uint32_t>& iBuffer, Asset& asset, const fastgltf::Primitive& primitive);
	static void loadModel(D3D* d3d, ID3D12GraphicsCommandList2* cmdList, Asset& asset, const fastgltf::Primitive& primitive, Model* model);
	static string loadTexture(Asset& asset, const size_t textureIndex);
	static void loadPrimitive(D3D* d3d, ID3D12GraphicsCommandList2* cmdList, Asset& asset, const fastgltf::Primitive& primitive, string modelNameExtensionless, fastgltf::Node& node, GLTFLoadArgs args, string id, size_t meshIndex, size_t primitiveIndex);
	static void loadNode(D3D* d3d, ID3D12GraphicsCommandList2* cmdList, Asset& asset, string modelNameExtensionless, fastgltf::Node& node, GLTFLoadArgs args);
	static void loadSplitModel(D3D* d3d, ID3D12GraphicsCommandList2* cmdList, string name, GLTFLoadArgs& args);
	static void loadModelsFromNode(D3D* d3d, ID3D12GraphicsCommandList2* cmdList, Asset& asset, string modelNameExtensionless, fastgltf::Node& node, vector<std::shared_ptr<Model>>& modelList);

	static fastgltf::Parser ms_parser;
	static bool ms_initialisedParser;
	static std::mutex ms_batchAddMutex;
};

