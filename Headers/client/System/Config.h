//
// Created by fionaw on 28/09/2025.
//

#ifndef PT_CONFIG_H
#define PT_CONFIG_H

#include <cstdint>
#include <dxgiformat.h>
#include <windows.h>

struct SettingsSystem
{
    uint32_t RtvWidth = 1020;
    uint32_t RtvHeight = 575;

    uint32_t WindowAppGuiWidth = 340;
    uint32_t WindowEngineGuiWidth = 340;

    bool VSyncEnabled = false;
    bool DsvEnabled = false;

    DXGI_FORMAT RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM; // TODO: Increase to Rgba16f?
};

class Config
{
public:
    static void ParseCommandLineArgs(LPSTR args);
    static SettingsSystem& GetSystem() { return ms_settingsSystem; }

private:
    static SettingsSystem ms_settingsSystem;
};


#endif //PT_CONFIG_H