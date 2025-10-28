//
// Created by fionaw on 28/09/2025.
//

#ifndef PT_CONFIG_H
#define PT_CONFIG_H

#include <cstdint>
#include <windows.h>

struct SettingsSystem
{
    uint32_t RtvWidth = 1020;
    uint32_t RtvHeight = 575;

    uint32_t WindowAppGuiWidth = 340;
    uint32_t WindowEngineGuiWidth = 340;

    bool VSyncEnabled = false;
};

struct SettingsRender
{
    bool PathTracingRngPaused = false;
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