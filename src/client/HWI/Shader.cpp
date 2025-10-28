//
// Created by fiona on 25/09/2025.
//

#include "HWI/Shader.h"

#include "Helper.h"
#include "System/Config.h"
#include "System/FileHelper.h"

#ifdef _DEBUG
#include "System/HotReloader.h"
#endif

inline ComPtr<IDxcBlob> CompileShaderDXC(
    const std::wstring& filePath,
    LPCWSTR entryPoint,
    LPCWSTR targetProfile,
    UINT compileFlags)
{
    HMODULE dxCompilerDLL = LoadLibrary("dxcompiler.dll");
    if (!dxCompilerDLL) {
        DWORD err = GetLastError();
        std::cout << "LoadLibrary failed: " << err << "\n";
    } else {
        std::cout << "dxcompiler.dll loaded successfully!\n";
    }

    // Get DxcCreateInstance function
    auto DxcCreateInstanceFn = reinterpret_cast<HRESULT(__stdcall*)(REFCLSID, REFIID, LPVOID*)>(
        GetProcAddress(dxCompilerDLL, "DxcCreateInstance"));
    if (!DxcCreateInstanceFn) {
        std::cerr << "Failed to get DxcCreateInstance\n";
    }

    // Create DXC objects
    ComPtr<IDxcCompiler3> compiler;
    ComPtr<IDxcLibrary> library;
    ComPtr<IDxcIncludeHandler> includeHandler;
    ComPtr<IDxcUtils> utils;
    try {
        V(DxcCreateInstanceFn(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)));
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        OutputDebugStringA("Unknown exception in DxcCreateInstance");
    }

    V(DxcCreateInstanceFn(CLSID_DxcLibrary, IID_PPV_ARGS(&library)));
    V(DxcCreateInstanceFn(CLSID_DxcUtils, IID_PPV_ARGS(&utils)));
    V(library->CreateIncludeHandler(&includeHandler));

    const auto shaderBytes = FileHelper::ReadFileToByteVector(filePath);

    DxcBuffer buffer;
    buffer.Ptr = shaderBytes.data();
    buffer.Size = shaderBytes.size();
    buffer.Encoding = DXC_CP_UTF8; // or DXC_CP_ACP if ASCII

    const std::wstring shadersPath = FileHelper::GetAssetsPath() + L"/Shaders/";
    const std::wstring dualIncludePath = FileHelper::GetAssetsPath() + L"/Shaders/DualIncludes/";

    ComPtr<IDxcResult> result;
    const wchar_t* args[] = { L"-E", entryPoint, L"-T", targetProfile, L"-I", dualIncludePath.c_str(), L"-I", shadersPath.c_str() };
    if (FAILED(compiler->Compile(&buffer, args, _countof(args), includeHandler.Get(), IID_PPV_ARGS(&result)))) {
        std::cerr << "Shader compile failed\n";
        return nullptr;
    }

    // Get compiled blob
    ComPtr<IDxcBlobUtf8> errors;
    if (FAILED(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr))) {
        std::cerr << "Failed to get shader errors\n";
        return nullptr;
    }
    if (errors && errors->GetStringLength() > 0) {
        std::cout << "Shader compile warnings/errors:\n" << errors->GetStringPointer() << "\n";
    }

    ComPtr<IDxcBlob> vertexShaderBlob;
    if (FAILED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&vertexShaderBlob), nullptr))) {
        std::cerr << "Failed to get compiled shader\n";
        return nullptr;
    }

    return vertexShaderBlob;
}

void Shader::InitVsPs(LPCWSTR vs, LPCWSTR ps, D3D12_INPUT_LAYOUT_DESC ild, ID3D12Device* device, ID3D12RootSignature* rootSig)
{
#if defined(_DEBUG)
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    std::wstring vsPath = FileHelper::GetAssetShaderFullPath(vs);
    std::wstring psPath = FileHelper::GetAssetShaderFullPath(ps);

    ComPtr<IDxcBlob> vertexShader = CompileShaderDXC(vsPath.c_str(), L"VSMain", L"vs_6_6", compileFlags);
    ComPtr<IDxcBlob> pixelShader = CompileShaderDXC(psPath.c_str(), L"PSMain", L"ps_6_6", compileFlags);
    
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = ild;
    psoDesc.pRootSignature = rootSig;
    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    //psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.DepthStencilState.DepthEnable = Config::GetSystem().DsvEnabled ? TRUE : FALSE;
    psoDesc.DepthStencilState.StencilEnable = Config::GetSystem().DsvEnabled ? TRUE : FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    V(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pso)));

#ifdef _DEBUG
    if (m_assignedToHotReload)
        return;
    HotReloader::AssignShaderVsPs(vs, ps, this, ild, rootSig);
    m_assignedToHotReload = true;
#endif
}

void Shader::InitCs(const LPCWSTR cs, ID3D12Device* device, ID3D12RootSignature* rootSig)
{
#if defined(_DEBUG)
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    std::wstring csPath = FileHelper::GetAssetShaderFullPath(cs);
    ComPtr<IDxcBlob> computeShader = CompileShaderDXC(csPath.c_str(), L"CSMain", L"cs_6_6", compileFlags);

    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = rootSig;
    psoDesc.CS = { computeShader->GetBufferPointer(), computeShader->GetBufferSize() };
    V(device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_pso)));

#ifdef _DEBUG
    if (m_assignedToHotReload)
        return;
    HotReloader::AssignShaderCs(cs, this, rootSig);
    m_assignedToHotReload = true;
#endif
}