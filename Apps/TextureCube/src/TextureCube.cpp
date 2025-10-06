#include "Apps/TextureCube/Headers/TextureCube.h"
#include "System/Win32App.h"
#include <dxcapi.h>

#include "Headers/Helper.h"
#include "HWI/D3D.h"
#include "System/Config.h"
#include "System/FileHelper.h"
#include "System/Gui.h"
#include "DualIncludes/CBV.h"
#include "HWI/Material.h"

TextureCube::TextureCube()
    : m_AspectRatio(0),
      m_vertexBufferView()
{
}

void TextureCube::OnInit(D3D* d3d)
{
    m_AspectRatio = static_cast<float>(Config::GetSystem().WindowWidth) / static_cast<float>(Config::GetSystem().
        WindowHeight);

    m_camera.Init({}, {});

    loadAssets(d3d);
}

void TextureCube::OnUpdate(D3D* d3d)
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
    XMFLOAT2 uv;
};

void TextureCube::loadAssets(D3D* d3d)
{
    ID3D12Device* device = d3d->GetDevice();
    ComPtr<ID3D12GraphicsCommandList> cmdList = d3d->GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    std::shared_ptr<RootSig> rootSig = std::make_shared<RootSig>();
    std::shared_ptr<Texture> tex = std::make_shared<Texture>();
    std::shared_ptr<Shader> shader = std::make_shared<Shader>();
    std::shared_ptr<Transform> transform = std::make_shared<Transform>();
    std::shared_ptr<Material> mat = std::make_shared<Material>();

    m_cube.Init(transform, tex, shader, rootSig, nullptr, mat);

    m_heap.Init(device, 256);

    mat->Init(&m_heap);
    mat->AddCBV(device, &m_heap, sizeof(CbvMatrices));

    // Init Root Sig
    {
        CD3DX12_ROOT_PARAMETER1 params[2];
        CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
        params[0].InitAsDescriptorTable(1, &ranges[0]);
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        params[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);

        D3D12_STATIC_SAMPLER_DESC samplers[1];
        samplers[0] = {};
        samplers[0].Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        samplers[0].ShaderRegister = 0;

        rootSig->Init(params, _countof(params), samplers, _countof(samplers), device);
    }

    // Init Shader/PSO
    {
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
        };
        D3D12_INPUT_LAYOUT_DESC ild = {inputElementDescs, _countof(inputElementDescs)};

        shader->InitVsPs(L"Basic3D_TexVS.hlsl", L"Basic3D_TexPS.hlsl", ild, device, rootSig->Get());
    }

    // Create the vertex buffer.
    {
        Vertex cubeVertices[] =
        {
            // +X face
            {{0.25f, -0.25f, -0.25f}, {1, 0, 0, 1}, {1, 1}}, // bottom-right
            {{0.25f, 0.25f, -0.25f}, {1, 0, 0, 1}, {1, 0}}, // top-right
            {{0.25f, 0.25f, 0.25f}, {1, 0, 0, 1}, {0, 0}}, // top-left

            {{0.25f, -0.25f, -0.25f}, {1, 0, 0, 1}, {1, 1}}, // bottom-right
            {{0.25f, 0.25f, 0.25f}, {1, 0, 0, 1}, {0, 0}}, // top-left
            {{0.25f, -0.25f, 0.25f}, {1, 0, 0, 1}, {0, 1}}, // bottom-left

            // -X face
            {{-0.25f, -0.25f, 0.25f}, {-1, 0, 0, 1}, {1, 1}}, // bottom-right
            {{-0.25f, 0.25f, 0.25f}, {-1, 0, 0, 1}, {1, 0}}, // top-right
            {{-0.25f, 0.25f, -0.25f}, {-1, 0, 0, 1}, {0, 0}}, // top-left

            {{-0.25f, -0.25f, 0.25f}, {-1, 0, 0, 1}, {1, 1}}, // bottom-right
            {{-0.25f, 0.25f, -0.25f}, {-1, 0, 0, 1}, {0, 0}}, // top-left
            {{-0.25f, -0.25f, -0.25f}, {-1, 0, 0, 1}, {0, 1}}, // bottom-left

            // +Y face
            {{-0.25f, 0.25f, -0.25f}, {0, 1, 0, 1}, {0, 1}}, // bottom-left
            {{-0.25f, 0.25f, 0.25f}, {0, 1, 0, 1}, {0, 0}}, // top-left
            {{0.25f, 0.25f, 0.25f}, {0, 1, 0, 1}, {1, 0}}, // top-right

            {{-0.25f, 0.25f, -0.25f}, {0, 1, 0, 1}, {0, 1}}, // bottom-left
            {{0.25f, 0.25f, 0.25f}, {0, 1, 0, 1}, {1, 0}}, // top-right
            {{0.25f, 0.25f, -0.25f}, {0, 1, 0, 1}, {1, 1}}, // bottom-right

            // -Y face
            {{-0.25f, -0.25f, 0.25f}, {0, -1, 0, 1}, {0, 1}}, // bottom-left
            {{-0.25f, -0.25f, -0.25f}, {0, -1, 0, 1}, {0, 0}}, // top-left
            {{0.25f, -0.25f, -0.25f}, {0, -1, 0, 1}, {1, 0}}, // top-right

            {{-0.25f, -0.25f, 0.25f}, {0, -1, 0, 1}, {0, 1}}, // bottom-left
            {{0.25f, -0.25f, -0.25f}, {0, -1, 0, 1}, {1, 0}}, // top-right
            {{0.25f, -0.25f, 0.25f}, {0, -1, 0, 1}, {1, 1}}, // bottom-right

            // +Z face
            {{-0.25f, -0.25f, 0.25f}, {0, 0, 1, 1}, {0, 1}}, // bottom-left
            {{0.25f, -0.25f, 0.25f}, {0, 0, 1, 1}, {1, 1}}, // bottom-right
            {{0.25f, 0.25f, 0.25f}, {0, 0, 1, 1}, {1, 0}}, // top-right

            {{-0.25f, -0.25f, 0.25f}, {0, 0, 1, 1}, {0, 1}}, // bottom-left
            {{0.25f, 0.25f, 0.25f}, {0, 0, 1, 1}, {1, 0}}, // top-right
            {{-0.25f, 0.25f, 0.25f}, {0, 0, 1, 1}, {0, 0}}, // top-left

            // -Z face
            {{0.25f, -0.25f, -0.25f}, {0, 0, -1, 1}, {1, 1}}, // bottom-right
            {{-0.25f, -0.25f, -0.25f}, {0, 0, -1, 1}, {0, 1}}, // bottom-left
            {{-0.25f, 0.25f, -0.25f}, {0, 0, -1, 1}, {0, 0}}, // top-left

            {{0.25f, -0.25f, -0.25f}, {0, 0, -1, 1}, {1, 1}}, // bottom-right
            {{-0.25f, 0.25f, -0.25f}, {0, 0, -1, 1}, {0, 0}}, // top-left
            {{0.25f, 0.25f, -0.25f}, {0, 0, -1, 1}, {1, 0}}, // top-right
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
        CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
        V(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
        memcpy(pVertexDataBegin, cubeVertices, sizeof(cubeVertices));
        m_vertexBuffer->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(Vertex);
        m_vertexBufferView.SizeInBytes = vertexBufferSize;
    }

    tex->Init(d3d->GetDevice(), cmdList.Get(), FileHelper::GetAssetTextureFullPath(L"TestTex.png"),
               DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    mat->AddSRV(device, &m_heap, tex.get());

    tex->Transition(cmdList.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    V(cmdList->Close());
    d3d->ExecuteCommandList(cmdList.Get());
    d3d->Flush();
}

void TextureCube::populateCommandList(const D3D* d3d, ID3D12GraphicsCommandList* cmdList)
{
    float fov = 60.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    CbvMatrices matrices = {};
    matrices.V = m_camera.GetViewMatrix();
    matrices.P = XMMatrixPerspectiveFovLH(XMConvertToRadians(fov), m_AspectRatio, nearPlane, farPlane);

    ID3D12Resource* rtv = d3d->GetCurrRTV();

    // Render at offset for ImGui
    CD3DX12_VIEWPORT viewport(static_cast<float>(Config::GetSystem().WindowImGuiWidth), 0.0f,
                              static_cast<float>(Config::GetSystem().WindowWidth),
                              static_cast<float>(Config::GetSystem().WindowHeight));
    CD3DX12_RECT scissorRect(Config::GetSystem().WindowImGuiWidth, 0,
                             Config::GetSystem().WindowWidth + Config::GetSystem().WindowImGuiWidth,
                             Config::GetSystem().WindowHeight);

    cmdList->RSSetViewports(1, &viewport);
    cmdList->RSSetScissorRects(1, &scissorRect);

    m_heap.SetHeap(cmdList);

    m_cube.GetTransform()->Rotate({0, 0.1, 0});

    // Indicate that the back buffer will be used as a render target.
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(rtv, D3D12_RESOURCE_STATE_PRESENT,
                                                        D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmdList->ResourceBarrier(1, &barrier);

    // Note: HERE AS WELL
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(d3d->GetRtvHeapStart(), d3d->GetFrameIndex(), d3d->GetRtvDescriptorSize());
    cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    constexpr float clearColor[] = {0.0f, 0.2f, 0.4f, 1.0f};
    cmdList->ClearRenderTargetView(rtvHandle, clearColor, 1, &scissorRect);

    m_cube.Render(cmdList, matrices);
    cmdList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    cmdList->DrawInstanced(m_vertexCount, 1, 0, 0);

    Gui::Render(cmdList);

    // Indicate that the back buffer will now be used to present.
    auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(rtv, D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                         D3D12_RESOURCE_STATE_PRESENT);
    cmdList->ResourceBarrier(1, &barrier2);

    V(cmdList->Close());
}

// Helper function for setting the window's title text.
void TextureCube::setCustomWindowText(LPCWSTR text) const
{
    std::wstring windowText = m_title + L": " + text;
    SetWindowText(Win32App::GetHwnd(), wstringToString(windowText).c_str());
}

// Helper function for parsing any supplied command line args.
_Use_decl_annotations_

void TextureCube::ParseCommandLineArgs(WCHAR* argv[], int argc)
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
