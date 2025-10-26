//
// Created by fionaw on 26/10/2025.
//

#include "System/Engine.h"

#include "Apps/App.h"
#include "HWI/D3D.h"
#include "System/Config.h"
#include "System/FileHelper.h"
#include "System/Gui.h"
#include "System/HotReloader.h"
#include "System/Input.h"
#include "System/TextureLoader.h"

Engine::Engine(App* pSample, const HWND hWnd, const UINT windowWidth)
{
    m_d3d = std::make_unique<D3D>();
    m_d3d->Init(windowWidth, Config::GetSystem().WindowHeight);
    TextureLoader::Init(m_d3d.get(), FileHelper::GetAssetsPath() + L"/Shaders");

    pSample->OnInit(m_d3d.get());

    Gui::Init(hWnd, m_d3d->GetDevice(), 3);
}

void Engine::Frame(const HWND hWnd)
{
    if (App* pSample = reinterpret_cast<App*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)))
    {
        const ComPtr<ID3D12GraphicsCommandList> cmdList = m_d3d->GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT);

        pSample->OnUpdate(m_d3d.get(), cmdList.Get());

        m_d3d->ExecuteCommandList(cmdList.Get());
        m_d3d->Present();
    }

    Input::ProgressFrame();

    m_clock.Tick();

#ifdef _DEBUG
    HotReloader::CheckFiles(m_d3d.get());
#endif
}
