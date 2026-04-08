//
// Created by fiona on 21/01/2026.
//

#ifndef CHERRYPIP_IMGUIUTILS_H
#define CHERRYPIP_IMGUIUTILS_H
#include "imgui.h"

class ImGuiUtils
{
public:
    static void FixWidthOnNext(const char* label);
    static void FixWidthOnNext(const char* label, const char* field, bool& needTooltip);
    static bool FwInputFloat(const char* label, float* value, float step = 0.0f, float stepFast = 0.0f,
                             const char* format = "%.3f", ImGuiInputTextFlags flags = ImGuiInputTextFlags_None);
    static bool FwInputInt(const char* label, int* value, int step = 1, int stepFast = 100,
                           ImGuiInputTextFlags flags = ImGuiInputTextFlags_None);
    static bool FwInputUInt(const char* label, uint32_t* value, int step = 1, int stepFast = 100,
                            ImGuiInputTextFlags flags = ImGuiInputTextFlags_None);
    static bool FwDragFloat(const char* label, float* value, float speed = 1.0f, float min = 0.0f, float max = 0.0f,
                            const char* format = "%.3f",
                            ImGuiInputTextFlags flags = ImGuiInputTextFlags_None);
    static bool FwDragInt(const char* label, int* value, float speed = 1.0f, int min = 0.0f, int max = 0.0f,
                          const char* format = "%d",
                          ImGuiInputTextFlags flags = ImGuiInputTextFlags_None);
    static bool FwInputFloat3(const char* label, float* value, const char* format = "%.3f",
                              ImGuiInputTextFlags flags = ImGuiInputTextFlags_None);
    static bool FwColorEdit3(const char* label, float* value, ImGuiInputTextFlags flags = ImGuiInputTextFlags_None);
    static bool FwColorEdit4(const char* label, float* value, ImGuiInputTextFlags flags = ImGuiInputTextFlags_None);
    static bool BeginComboWithTooltip(const char* label, const char* preview, ImGuiComboFlags flags = 0);
};


#endif //CHERRYPIP_IMGUIUTILS_H
