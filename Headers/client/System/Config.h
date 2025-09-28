//
// Created by fionaw on 28/09/2025.
//

#ifndef PT_CONFIG_H
#define PT_CONFIG_H

#include <cstdint>
#include <windows.h>

struct SettingsSystem
{
    uint32_t WindowWidth = 1020;
    uint32_t WindowHeight = 575;

    uint32_t WindowImGuiWidth = 340;
};

class Config
{
public:
    static void ParseCommandLineArgs(LPSTR args);
    static SettingsSystem GetSystem() { return ms_settingsSystem; }

private:
    static SettingsSystem ms_settingsSystem;
};


#endif //PT_CONFIG_H