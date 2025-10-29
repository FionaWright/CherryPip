//
// Created by fionaw on 28/09/2025.
//

#include "System/Config.h"

#include <iostream>
#include <sstream>
#include <unordered_map>

SettingsSystem Config::ms_settingsSystem;

template <typename Out>
void split(const std::string &s, const char delim, Out result) {
    std::istringstream iss(s);
    std::string item;
    while (std::getline(iss, item, delim)) {
        *result++ = item;
    }
}

std::vector<std::string> split(const std::string &s, const char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

inline void LinkBool(const std::unordered_map<std::string, std::string>& map, bool& variable, const char* key)
{
    variable = map.contains(key) && map.at(key) != "false";
}

inline void LinkUInt(const std::unordered_map<std::string, std::string>& map, uint32_t& variable, const char* key)
{
    if (map.contains(key))
        variable = std::stoi(map.at(key));
}

void Config::ParseCommandLineArgs(const LPSTR args)
{
    std::unordered_map<std::string, std::string> argsMap;

    const std::vector<std::string> splitArgs = split(args, ' ');

    for (int i = 0; i < splitArgs.size(); i++)
    {
        std::string arg = splitArgs[i];
        const int eqIdx = arg.find('=');
        if (eqIdx == std::string::npos)
        {
            argsMap.insert({arg, ""});
            continue;
        }

        const std::string key = arg.substr(0, eqIdx);
        const std::string value = arg.substr(eqIdx + 1);
        argsMap.insert({key, value});
    }

    std::cout << "Window Width: " << ms_settingsSystem.RtvWidth << std::endl;

    LinkUInt(argsMap, ms_settingsSystem.RtvWidth, "--window_width");
    LinkUInt(argsMap, ms_settingsSystem.RtvHeight, "--window_height");
    LinkUInt(argsMap, ms_settingsSystem.DefaultAppIdx, "--app");
    LinkBool(argsMap, ms_settingsSystem.VSyncEnabled, "--vsync");

    std::cout << "Window Width: " << ms_settingsSystem.RtvWidth << std::endl;
}