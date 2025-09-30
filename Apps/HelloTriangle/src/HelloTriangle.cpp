#include "Apps/HelloTriangle/Headers/HelloTriangle.h"
#include "System/Win32App.h"
#include <dxcapi.h>

#include "../../../Headers/Helper.h"
#include "HWI/D3D.h"
#include "System/Config.h"
#include "System/FileHelper.h"

HelloTriangle::HelloTriangle()
    : m_AspectRatio(0),
      m_vertexBufferView()
{
}

void HelloTriangle::OnInit(D3D* d3d)
{
    m_AspectRatio = static_cast<float>(Config::GetSystem().WindowWidth) / static_cast<float>(Config::GetSystem().WindowHeight);

    loadAssets(d3d->GetDevice());

    d3d->Flush();
}

void HelloTriangle::OnUpdate(D3D* d3d)
{
    const ComPtr<ID3D12GraphicsCommandList> cmdList = d3d->GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    populateCommandList(d3d, cmdList.Get());

    d3d->ExecuteCommandList(cmdList.Get());
    d3d->Present();
}

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT4 color;
};

void HelloTriangle::loadAssets(ID3D12Device* device)
{
    // Create an empty root signature.
    {
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        V(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        V(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {
        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        m_shader.InitVsPs(L"Basic_Color.hlsl", L"Basic_Color.hlsl", { inputElementDescs, _countof(inputElementDescs)}, device, m_rootSignature.Get());
    }

    // Create the vertex buffer.
    {
        // Define the geometry for a triangle.
        Vertex triangleVertices[] =
        {
            { { 0.0f, 0.25f * m_AspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
            { { 0.25f, -0.25f * m_AspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
            { { -0.25f, -0.25f * m_AspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
        };

        constexpr UINT vertexBufferSize = sizeof(triangleVertices);

        auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto buffer = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

        // Note: using upload heaps to transfer static data like vert buffers is not
        // recommended. Every time the GPU needs it, the upload heap will be marshalled
        // over. Please read up on Default Heap usage. An upload heap is used here for
        // code simplicity and because there are very few verts to actually transfer.
        V(device->CreateCommittedResource(
            &prop,
            D3D12_HEAP_FLAG_NONE,
            &buffer,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_vertexBuffer)));

        // Copy the triangle data to the vertex buffer.
        UINT8* pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        V(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
        memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
        m_vertexBuffer->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(Vertex);
        m_vertexBufferView.SizeInBytes = vertexBufferSize;
    }
}

void HelloTriangle::populateCommandList(const D3D* d3d, ID3D12GraphicsCommandList* cmdList) const
{
    ID3D12Resource* rtv = d3d->GetCurrRTV();

    CD3DX12_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(Config::GetSystem().WindowWidth), static_cast<float>(Config::GetSystem().WindowHeight));
    CD3DX12_RECT scissorRect(0, 0, static_cast<LONG>(Config::GetSystem().WindowWidth), static_cast<LONG>(Config::GetSystem().WindowHeight));

    // Set necessary state.
    cmdList->SetGraphicsRootSignature(m_rootSignature.Get());
    cmdList->RSSetViewports(1, &viewport);
    cmdList->RSSetScissorRects(1, &scissorRect);
    cmdList->SetPipelineState(m_shader.GetPSO());

    // Indicate that the back buffer will be used as a render target.
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(rtv, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmdList->ResourceBarrier(1, &barrier);

    // Note: HERE AS WELL
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(d3d->GetRtvHeapStart(), d3d->GetFrameIndex(), d3d->GetRtvDescriptorSize());
    cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    cmdList->DrawInstanced(3, 1, 0, 0);

    // Indicate that the back buffer will now be used to present.
    auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(rtv, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    cmdList->ResourceBarrier(1, &barrier2);

    V(cmdList->Close());
}

// Helper function for setting the window's title text.
void HelloTriangle::setCustomWindowText(LPCWSTR text) const
{
    std::wstring windowText = m_title + L": " + text;
    SetWindowText(Win32App::GetHwnd(), wstringToString(windowText).c_str());
}

// Helper function for parsing any supplied command line args.
_Use_decl_annotations_
void HelloTriangle::ParseCommandLineArgs(WCHAR* argv[], int argc)
{
    for (int i = 1; i < argc; ++i)
    {
        if (_wcsnicmp(argv[i], L"-warp", wcslen(argv[i])) == 0 ||
            _wcsnicmp(argv[i], L"/warp", wcslen(argv[i])) == 0)
        {
            //m_UseWarpDevice = true;
            m_title = m_title + L" (WARP)";
        }
    }
}