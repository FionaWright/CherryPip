//
// Created by fionaw on 28/09/2025.
//

#ifndef PT_CONFIG_H
#define PT_CONFIG_H

#include <cstdint>
#include <dxgiformat.h>
#include <windows.h>

#define NUM_FRAMES_IN_FLIGHT 3

struct SettingsSystem
{
    uint32_t RtvWidth = 1020;
    uint32_t RtvHeight = 575;

    uint32_t WindowAppGuiWidth = 340;
    uint32_t WindowEngineGuiWidth = 340;

    uint32_t DefaultAppIdx = 0;
    uint32_t DefaultSceneIdx = 0;

    bool VSyncEnabled = false;
    bool ForceSyncCpuGpu = true;
    bool AppGuiEnabled = true;

    bool ProfilingEnabled = true;
};

struct SettingsRender
{
    DXGI_FORMAT RtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    FLOAT RtvClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    float FoV = 60.0f;
    float NearPlane = 0.1f;
    float FarPlane = 100.0f;
};

class Config
{
public:
    static void ParseCommandLineArgs(LPSTR args);
    static SettingsSystem& GetSystem() { return ms_settingsSystem; }
    static SettingsRender& GetRender() { return ms_settingsRender; }

private:
    static SettingsSystem ms_settingsSystem;
    static SettingsRender ms_settingsRender;
};


#endif //PT_CONFIG_H