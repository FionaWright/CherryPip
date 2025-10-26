//
// Created by fionaw on 28/09/2025.
//

#ifndef PT_GUI_H
#define PT_GUI_H
#include <d3d12.h>
#include <wrl/client.h>

struct ImVec2;

class Gui
{
public:
    static void Init(HWND hwnd, ID3D12Device* device, int framesInFlight);
    static void BeginFrame();

    static void BeginWindow(const char* name, const ImVec2& pos, const ImVec2& size);
    static void EndWindow();

    static void RenderAllWindows(ID3D12GraphicsCommandList* cmdList);

private:
    static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> ms_cbvSrvUavHeap;
};


#endif //PT_GUI_H