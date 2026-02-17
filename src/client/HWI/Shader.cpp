//
// Created by fiona on 25/09/2025.
//

#include "System/pch.h"
#include "HWI/Shader.h"
#include "../../../Headers/client/Helper.h"
#include "System/Config.h"
#include "System/FileHelper.h"

#ifdef _DEBUG
#include "../../../Headers/client/Debug/HotReloader.h"
#endif

inline ComPtr<IDxcBlob> CompileShaderDXC(
    const std::wstring& filePath,
    const LPCWSTR entryPoint,
    LPCWSTR targetProfile,
    UINT compileFlags,
    std::vector<const WCHAR*> args)
{
    HMODULE dxCompilerDLL = LoadLibrary("dxcompiler.dll");
    if (!dxCompilerDLL)
    {
        CherryPrint("LoadLibrary failed: " << GetLastError());
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
    try
    {
        V(DxcCreateInstanceFn(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)));
    }
    catch (const std::exception& e)
    {
        CherryPrint(e.what());
    }
    catch (...)
    {
        CherryPrint("Unknown exception in DxcCreateInstance");
    }

    V(DxcCreateInstanceFn(CLSID_DxcLibrary, IID_PPV_ARGS(&library)));
    V(DxcCreateInstanceFn(CLSID_DxcUtils, IID_PPV_ARGS(&utils)));
    V(library->CreateIncludeHandler(&includeHandler));

    const auto shaderBytes = FileHelper::ReadFileToByteVector(filePath);

    DxcBuffer buffer;
    buffer.Ptr = shaderBytes.data();
    buffer.Size = shaderBytes.size();
    buffer.Encoding = DXC_CP_UTF8; // or DXC_CP_ACP if ASCII

    const std::wstring shadersPath = FileHelper::GetShadersPath();
    const std::wstring dualIncludePath = FileHelper::GetShadersPath() + L"DualIncludes/";

    ComPtr<IDxcResult> result;
    const std::vector<const wchar_t*> stdArgs = { L"-E", entryPoint, L"-T", targetProfile, L"-I", dualIncludePath.c_str(), L"-I", shadersPath.c_str() };
    args.insert(args.end(), stdArgs.begin(), stdArgs.end());
    if (FAILED(compiler->Compile(&buffer, args.data(), args.size(), includeHandler.Get(), IID_PPV_ARGS(&result))))
    {
        CherryPrint("Shader compile failed");
        return nullptr;
    }

    // Get compiled blob
    ComPtr<IDxcBlobUtf8> errors;
    if (FAILED(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr)))
    {
        CherryPrint("Failed to get shader errors");
        return nullptr;
    }
    if (errors && errors->GetStringLength() > 0)
    {
        CherryPrint("Shader compile warnings/errors:\n" << errors->GetStringPointer());
    }

    ComPtr<IDxcBlob> vertexShaderBlob;
    if (FAILED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&vertexShaderBlob), nullptr)))
    {
        CherryPrint("Failed to get compiled shader");
        return nullptr;
    }

    return vertexShaderBlob;
}

void Shader::InitVsPs(LPCWSTR vs, LPCWSTR ps, D3D12_INPUT_LAYOUT_DESC ild, ID3D12Device* device, ID3D12RootSignature* rootSig, const bool dsvEnabled, const std::vector<const WCHAR*>& args, const uint32_t numRTVs, const D3D12_PRIMITIVE_TOPOLOGY_TYPE topology)
{
#if defined(_DEBUG)
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    if (m_pso)
    {
        m_pso = nullptr;
    }

    std::wstring vsPath = FileHelper::GetAssetShaderFullPath(vs);
    std::wstring psPath = FileHelper::GetAssetShaderFullPath(ps);

    ComPtr<IDxcBlob> vertexShader = CompileShaderDXC(vsPath.c_str(), L"VSMain", L"vs_6_6", compileFlags, args);
    ComPtr<IDxcBlob> pixelShader = CompileShaderDXC(psPath.c_str(), L"PSMain", L"ps_6_6", compileFlags, args);
    
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
    psoDesc.DepthStencilState.DepthEnable = dsvEnabled ? TRUE : FALSE;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.StencilEnable = dsvEnabled ? TRUE : FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = topology;
    psoDesc.NumRenderTargets = numRTVs;
    for (int i = 0; i < numRTVs; i++)
        psoDesc.RTVFormats[i] = Config::GetRender().RtvFormat;
    psoDesc.SampleDesc.Count = 1;
    V(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pso)));

#ifdef _DEBUG
    if (m_assignedToHotReload)
    {
        HotReloader::UpdateShaderVsPs(vs, ps, this, ild, rootSig, dsvEnabled, args, numRTVs);
        return;
    }
    HotReloader::AssignShaderVsPs(vs, ps, this, ild, rootSig, dsvEnabled, args, numRTVs);
    m_assignedToHotReload = true;
#endif
}

void Shader::InitCs(const LPCWSTR cs, ID3D12Device* device, ID3D12RootSignature* rootSig, const std::vector<const WCHAR*>& args)
{
#if defined(_DEBUG)
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    std::wstring csPath = FileHelper::GetAssetShaderFullPath(cs);
    ComPtr<IDxcBlob> computeShader = CompileShaderDXC(csPath, L"CSMain", L"cs_6_6", compileFlags, args);

    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = rootSig;
    psoDesc.CS = { computeShader->GetBufferPointer(), computeShader->GetBufferSize() };
    V(device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_pso)));

#ifdef _DEBUG
    if (m_assignedToHotReload)
    {
        HotReloader::UpdateShaderCs(cs, this, rootSig);
        return;
    }
    HotReloader::AssignShaderCs(cs, this, rootSig);
    m_assignedToHotReload = true;
#endif
}