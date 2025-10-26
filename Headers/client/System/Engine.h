//
// Created by fionaw on 26/10/2025.
//

#ifndef PT_ENGINE_H
#define PT_ENGINE_H
#include <memory>

#include "HighResolutionClock.h"
#include "Apps/App.h"

class D3D;

class Engine
{
public:
    Engine(App* pSample, HWND hWnd, UINT windowWidth, UINT windowHeight);
    void Frame(HWND hWnd);
    void Render(App* pSample);
    void RenderGUI();
    void CalculateFPS(double deltaTime);

private:
    std::unique_ptr<D3D> m_d3d;
    HighResolutionClock m_clock;

    double m_fps10ms = 0.0, m_fps50ms = 0.0, m_fps100ms = 0.0;
    double m_fpsTimeSinceUpdate10ms = 0.0, m_fpsTimeSinceUpdate50ms = 0.0, m_fpsTimeSinceUpdate100ms = 0.0;
    unsigned int m_fpsFramesSinceUpdate10ms = 0, m_fpsFramesSinceUpdate50ms = 0, m_fpsFramesSinceUpdate100ms = 0;
    std::vector<float> m_fpsGuiQueue;
};

#endif
