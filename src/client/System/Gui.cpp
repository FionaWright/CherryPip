//
// Created by fionaw on 28/09/2025.
//

#include "System/Gui.h"

#include "Helper.h"
#include "imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx12.h"
#include "System/Config.h"

ComPtr<ID3D12DescriptorHeap> Gui::ms_cbvSrvUavHeap;

void Gui::Init(const HWND hwnd, ID3D12Device* device, const int framesInFlight)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.Fonts->AddFontDefault();
    io.Fonts->Build();

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = 1 * framesInFlight;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    desc.NodeMask = 0;
    V(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&ms_cbvSrvUavHeap)));

    const auto hCpu = ms_cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart();
    const auto hGpu = ms_cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX12_Init(device, framesInFlight, Config::GetSystem().RTVFormat, ms_cbvSrvUavHeap.Get(), hCpu, hGpu);

    ImGui_ImplDX12_CreateDeviceObjects();
}

// Called once per frame / cmdList
void Gui::BeginFrame()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

// Called for each window within a cmdList
void Gui::BeginWindow(const char* name, const ImVec2& pos, const ImVec2& size)
{
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(size, ImGuiCond_Always);

    ImGui::Begin(name, nullptr, ImGuiWindowFlags_None);
    //ms_windowSize = ImGui::GetWindowSize();
}

// Called for each window within a cmdList
void Gui::EndWindow()
{
    ImGui::End();
}

// Called once per frame / cmdList
void Gui::RenderAllWindows(ID3D12GraphicsCommandList* cmdList)
{
    ImGui::Render();

    ID3D12DescriptorHeap* pDescHeap = ms_cbvSrvUavHeap.Get();
    cmdList->SetDescriptorHeaps(1, &pDescHeap);

    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);
}