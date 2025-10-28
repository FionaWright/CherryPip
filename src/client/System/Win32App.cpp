//
// Created by fiona on 22/09/2025.
//

#include "System/Win32App.h"
#include "Apps/App.h"

#include "HWI/D3D.h"
#include "System/FileHelper.h"
#include "System/Gui.h"
#include "System/Input.h"

#include "imgui/backends/imgui_impl_win32.h"
#include "System/Config.h"
#include "System/TextureLoader.h"

#ifdef _DEBUG
#include "System/HotReloader.h"
#endif

#include "Assets/Resources/Resources.h"

HWND Win32App::ms_hwnd = nullptr;
std::unique_ptr<Engine> Win32App::ms_engine = nullptr;

int Win32App::Run(const std::vector<App*>& apps, HINSTANCE hInstance, int nCmdShow)
{
    FileHelper::Init();

    // Initialize the window class.
    WNDCLASSEX windowClass = {0};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = "WindowClass";
    windowClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON)); // <-- your main icon
    windowClass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON)); // small icon for title bar
    RegisterClassEx(&windowClass);

    const uint32_t totalWindowWidth = Config::GetSystem().RtvWidth + Config::GetSystem().WindowAppGuiWidth +
        Config::GetSystem().WindowEngineGuiWidth;
    const uint32_t totalWindowHeight = Config::GetSystem().RtvHeight;
    RECT windowRect = {0, 0, static_cast<LONG>(totalWindowWidth), static_cast<LONG>(Config::GetSystem().RtvHeight)};
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window and store a handle to it.
    ms_hwnd = CreateWindow(
        windowClass.lpszClassName,
        "CherryPip",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr, // We have no parent window.
        nullptr, // We aren't using menus.
        hInstance,
        nullptr);

    if (!ms_hwnd)
    {
        MessageBox(nullptr, "Failed to create window!", "Error", MB_OK);
    }
    std::cout << "HWND = " << ms_hwnd << std::endl;

    ms_engine = std::make_unique<Engine>(apps, ms_hwnd, totalWindowWidth, totalWindowHeight);

    ShowWindow(ms_hwnd, nCmdShow);

    // Main sample loop.
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        // Process any messages in the queue.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Return this part of the WM_QUIT message to Windows.
    return static_cast<char>(msg.wParam);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main message handler for the sample.
LRESULT CALLBACK Win32App::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    switch (message)
    {
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
        {
            Input::AddKey(static_cast<KeyCode::Key>(wParam));
        }
        break;

    case WM_SYSKEYUP:
    case WM_KEYUP:
        {
            Input::RemoveKey(static_cast<KeyCode::Key>(wParam));
        }
        break;

    case WM_MOUSEMOVE:
        {
            const float x = static_cast<float>(static_cast<int>(static_cast<short>(LOWORD(lParam))));
            const float y = static_cast<float>(static_cast<int>(static_cast<short>(HIWORD(lParam))));

            Input::SetMousePos(XMFLOAT2(x, y));

            POINT clientToScreenPoint;
            clientToScreenPoint.x = static_cast<LONG>(x);
            clientToScreenPoint.y = static_cast<LONG>(y);
            ScreenToClient(hWnd, &clientToScreenPoint);

            XMFLOAT2 client;
            client.x = static_cast<float>(clientToScreenPoint.x);
            client.y = static_cast<float>(clientToScreenPoint.y);
            Input::SetMousePosClient(client);
        }
        break;

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
        {
            const bool lButton = (wParam & MK_LBUTTON) != 0;
            const bool rButton = (wParam & MK_RBUTTON) != 0;
            const bool mButton = (wParam & MK_MBUTTON) != 0;

            Input::SetMouseLeftState(lButton);
            Input::SetMouseRightState(rButton);
            Input::SetMouseMiddleState(mButton);
        }
        break;

    case WM_MOUSEWHEEL:
        {
            const float zDelta = static_cast<int>(static_cast<short>(HIWORD(wParam))) / static_cast<float>(WHEEL_DELTA);
            Input::SetMouseWheelDelta(zDelta);
        }
        break;

    case WM_PAINT:
        ms_engine->Frame();
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default: ;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}
