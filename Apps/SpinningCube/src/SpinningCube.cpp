#include "Apps/SpinningCube/Headers/SpinningCube.h"
#include "System/Win32App.h"
#include <dxcapi.h>

#include "../../../Headers/Helper.h"
#include "HWI/D3D.h"
#include "System/Config.h"
#include "System/FileHelper.h"
#include "System/Gui.h"

SpinningCube::SpinningCube()
    : m_AspectRatio(0),
      m_vertexBufferView()
{
}

void SpinningCube::OnInit(D3D* d3d)
{
    m_AspectRatio = static_cast<float>(Config::GetSystem().WindowWidth) / static_cast<float>(Config::GetSystem().WindowHeight);

    m_camera.Init({}, {});

    loadAssets(d3d);
}

void SpinningCube::OnUpdate(D3D* d3d)
{
    ComPtr<ID3D12GraphicsCommandList> cmdList = d3d->GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    populateCommandList(d3d, cmdList.Get());

    d3d->ExecuteCommandList(cmdList.Get());
    d3d->Present();

    m_camera.UpdateCamera();
}

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT4 normal;
};

struct CbvMatrices
{
    XMMATRIX M; // Model
    XMMATRIX MTI; // Model Transpose Inverse (For Normals)
    XMMATRIX V; // View
    XMMATRIX P; // Projection
};

void SpinningCube::loadAssets(D3D* d3d)
{
    ID3D12Device* device = d3d->GetDevice();
    ComPtr<ID3D12GraphicsCommandList> cmdList = d3d->GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    {
        m_descriptorIncSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 1 * c_FrameCount;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NodeMask = 0;

        V(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_cbvSrvUavHeap)));

        // ===

        size_t alignedSize = (sizeof(CbvMatrices) + 255) & ~255; // Ceilings the size to the nearest 256

        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC bufferDesc = {};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Width = alignedSize;
        bufferDesc.Height = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.MipLevels = 1;
        bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
        bufferDesc.SampleDesc.Count = 1;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_cbv));

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = m_cbv->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = static_cast<UINT>(alignedSize);

        auto cbvHandle = m_cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart();
        //cbvHandle.ptr += incrementSize * (heapStart);
        device->CreateConstantBufferView(&cbvDesc, cbvHandle);
    }

    {
        CD3DX12_ROOT_PARAMETER1 param;
        CD3DX12_DESCRIPTOR_RANGE1 range;
        range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
        param.InitAsDescriptorTable(1, &range);

        m_rootSig.Init(&param, 1, nullptr, 0, device);
    }

    {
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };
        D3D12_INPUT_LAYOUT_DESC ild = { inputElementDescs, _countof(inputElementDescs) };

        m_shaderNormals.InitVsPs(L"Basic3D_NormalsVS.hlsl", L"Basic3D_NormalsPS.hlsl", ild, device, m_rootSig.Get());
    }

    // Create the vertex buffer.
    {
        Vertex cubeVertices[] =
        {
            // +X face
            { { 0.25f, -0.25f, -0.25f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
            { { 0.25f,  0.25f, -0.25f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
            { { 0.25f,  0.25f,  0.25f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
            { { 0.25f, -0.25f, -0.25f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
            { { 0.25f,  0.25f,  0.25f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
            { { 0.25f, -0.25f,  0.25f }, { 1.0f, 0.0f, 0.0f, 1.0f } },

            // -X face
            { { -0.25f, -0.25f,  0.25f }, { -1.0f, 0.0f, 0.0f, 1.0f } },
            { { -0.25f,  0.25f,  0.25f }, { -1.0f, 0.0f, 0.0f, 1.0f } },
            { { -0.25f,  0.25f, -0.25f }, { -1.0f, 0.0f, 0.0f, 1.0f } },
            { { -0.25f, -0.25f,  0.25f }, { -1.0f, 0.0f, 0.0f, 1.0f } },
            { { -0.25f,  0.25f, -0.25f }, { -1.0f, 0.0f, 0.0f, 1.0f } },
            { { -0.25f, -0.25f, -0.25f }, { -1.0f, 0.0f, 0.0f, 1.0f } },

            // +Y face
            { { -0.25f, 0.25f, -0.25f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
            { { -0.25f, 0.25f,  0.25f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
            { {  0.25f, 0.25f,  0.25f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
            { { -0.25f, 0.25f, -0.25f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
            { {  0.25f, 0.25f,  0.25f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
            { {  0.25f, 0.25f, -0.25f }, { 0.0f, 1.0f, 0.0f, 1.0f } },

            // -Y face
            { { -0.25f, -0.25f,  0.25f }, { 0.0f, -1.0f, 0.0f, 1.0f } },
            { { -0.25f, -0.25f, -0.25f }, { 0.0f, -1.0f, 0.0f, 1.0f } },
            { {  0.25f, -0.25f, -0.25f }, { 0.0f, -1.0f, 0.0f, 1.0f } },
            { { -0.25f, -0.25f,  0.25f }, { 0.0f, -1.0f, 0.0f, 1.0f } },
            { {  0.25f, -0.25f, -0.25f }, { 0.0f, -1.0f, 0.0f, 1.0f } },
            { {  0.25f, -0.25f,  0.25f }, { 0.0f, -1.0f, 0.0f, 1.0f } },

            // +Z face
            { { -0.25f, -0.25f, 0.25f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
            { {  0.25f, -0.25f, 0.25f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
            { {  0.25f,  0.25f, 0.25f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
            { { -0.25f, -0.25f, 0.25f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
            { {  0.25f,  0.25f, 0.25f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
            { { -0.25f,  0.25f, 0.25f }, { 0.0f, 0.0f, 1.0f, 1.0f } },

            // -Z face
            { {  0.25f, -0.25f, -0.25f }, { 0.0f, 0.0f, -1.0f, 1.0f } },
            { { -0.25f, -0.25f, -0.25f }, { 0.0f, 0.0f, -1.0f, 1.0f } },
            { { -0.25f,  0.25f, -0.25f }, { 0.0f, 0.0f, -1.0f, 1.0f } },
            { {  0.25f, -0.25f, -0.25f }, { 0.0f, 0.0f, -1.0f, 1.0f } },
            { { -0.25f,  0.25f, -0.25f }, { 0.0f, 0.0f, -1.0f, 1.0f } },
            { {  0.25f,  0.25f, -0.25f }, { 0.0f, 0.0f, -1.0f, 1.0f } },
        };

        m_vertexCount = _countof(cubeVertices);
        constexpr UINT vertexBufferSize = sizeof(cubeVertices);

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
        memcpy(pVertexDataBegin, cubeVertices, sizeof(cubeVertices));
        m_vertexBuffer->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(Vertex);
        m_vertexBufferView.SizeInBytes = vertexBufferSize;
    }

    cmdList->Close();
    d3d->ExecuteCommandList(cmdList.Get());
    d3d->Flush();
}

void SpinningCube::populateCommandList(D3D* d3d, ID3D12GraphicsCommandList* cmdList)
{
    ID3D12Resource* rtv = d3d->GetCurrRTV();

    // Render at offset for ImGui
    CD3DX12_VIEWPORT viewport(static_cast<float>(Config::GetSystem().WindowImGuiWidth), 0.0f, static_cast<float>(Config::GetSystem().WindowWidth), static_cast<float>(Config::GetSystem().WindowHeight));
    CD3DX12_RECT scissorRect(Config::GetSystem().WindowImGuiWidth, 0, Config::GetSystem().WindowWidth + Config::GetSystem().WindowImGuiWidth, Config::GetSystem().WindowHeight);

    // Set necessary state.
    cmdList->SetGraphicsRootSignature(m_rootSig.Get());
    cmdList->RSSetViewports(1, &viewport);
    cmdList->RSSetScissorRects(1, &scissorRect);
    cmdList->SetPipelineState(m_shaderNormals.GetPSO());

    ID3D12DescriptorHeap* heap = m_cbvSrvUavHeap.Get();
    cmdList->SetDescriptorHeaps(1, &heap);

    float fov = 60.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    m_transformCube.Rotate({0, 0.1f, 0});

    CbvMatrices matrices = {};
    matrices.M = m_transformCube.GetModelMatrix();
    matrices.MTI = XMMatrixInverse(nullptr, XMMatrixTranspose(matrices.M));
    matrices.V = m_camera.GetViewMatrix();
    matrices.P = XMMatrixPerspectiveFovLH(XMConvertToRadians(fov), m_AspectRatio, nearPlane, farPlane);

    void* dstData = nullptr;
    D3D12_RANGE readRange = {};
    m_cbv->Map(0, &readRange, &dstData);
    std::memcpy(dstData, &matrices, sizeof(CbvMatrices));
    m_cbv->Unmap(0, nullptr);

    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart();
    CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandle(gpuHandle, 0, m_descriptorIncSize);
    cmdList->SetGraphicsRootDescriptorTable(0, cbvHandle);

    // Indicate that the back buffer will be used as a render target.
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(rtv, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmdList->ResourceBarrier(1, &barrier);

    // Note: HERE AS WELL
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(d3d->GetRtvHeapStart(), d3d->GetFrameIndex(), d3d->GetRtvDescriptorSize());
    cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    cmdList->ClearRenderTargetView(rtvHandle, clearColor, 1, &scissorRect);
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    cmdList->DrawInstanced(m_vertexCount, 1, 0, 0);

    Gui::RenderAppSide(cmdList);

    // Indicate that the back buffer will now be used to present.
    auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(rtv, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    cmdList->ResourceBarrier(1, &barrier2);

    V(cmdList->Close());
}

// Helper function for setting the window's title text.
void SpinningCube::setCustomWindowText(LPCWSTR text) const
{
    std::wstring windowText = m_title + L": " + text;
    SetWindowText(Win32App::GetHwnd(), wstringToString(windowText).c_str());
}

// Helper function for parsing any supplied command line args.
_Use_decl_annotations_
void SpinningCube::ParseCommandLineArgs(WCHAR* argv[], int argc)
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