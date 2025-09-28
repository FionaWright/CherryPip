//
// Created by fionaw on 28/09/2025.
//

#ifndef PT_GUI_H
#define PT_GUI_H
#include <d3d12.h>
#include <wrl/client.h>

class Gui
{
public:
    static void Init(HWND hwnd, ID3D12Device* device, int framesInFlight);

    static void BeginFrame();
    static void Render(ID3D12GraphicsCommandList* cmdList);
    static void EndFrame(ID3D12GraphicsCommandList* cmdList);

private:
    static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> ms_cbvSrvUavHeap;
};


#endif //PT_GUI_H