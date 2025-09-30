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

HWND Win32App::m_hwnd = nullptr;
std::unique_ptr<D3D> Win32App::m_d3d = nullptr;

int Win32App::Run(App* pSample, HINSTANCE hInstance, int nCmdShow)
{
    FileHelper::Init();

    // Parse the command line parameters
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    pSample->ParseCommandLineArgs(argv, argc);
    LocalFree(argv);

    // Initialize the window class.
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = "HelloTriangleClass";
    RegisterClassEx(&windowClass);

    const uint32_t windowWidth = Config::GetSystem().WindowWidth + Config::GetSystem().WindowImGuiWidth;
    RECT windowRect = { 0, 0, static_cast<LONG>(windowWidth), static_cast<LONG>(Config::GetSystem().WindowHeight) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window and store a handle to it.
    m_hwnd = CreateWindow(
        windowClass.lpszClassName,
        "TEST WINDOW",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,        // We have no parent window.
        nullptr,        // We aren't using menus.
        hInstance,
        pSample);

    if (!m_hwnd)
    {
        MessageBox(nullptr, "Failed to create window!", "Error", MB_OK);
    }
    std::cout << "HWND = " << m_hwnd << std::endl;

    m_d3d = std::make_unique<D3D>();
    m_d3d->Init(windowWidth, Config::GetSystem().WindowHeight);
    TextureLoader::Init(m_d3d.get(), FileHelper::GetAssetsPath() + L"/Shaders");

    pSample->OnInit(m_d3d.get());

    ShowWindow(m_hwnd, nCmdShow);

    Gui::Init(m_hwnd, m_d3d->GetDevice(), 3);

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

    App* pSample = reinterpret_cast<App*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_CREATE:
        {
            // Save the HelloTriangle* passed in to CreateWindow.
            LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        }
        return 0;

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
        if (pSample)
        {
            pSample->OnUpdate(m_d3d.get());
        }
        Input::ProgressFrame();
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default: ;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}