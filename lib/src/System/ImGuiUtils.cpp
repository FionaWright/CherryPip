//
// Created by fiona on 21/01/2026.
//

#include "System/ImGuiUtils.h"

#define BONUS_WIDGET_WIDTH 20

template <typename T>
const char* Format(const char* fmt, const T value)
{
    static char buf[64];
    std::snprintf(buf, sizeof(buf), fmt, value);
    return buf;
}

template <typename T>
const char* FormatN(const char* fmt, const T* values, const int count)
{
    static char buf[128];
    static std::unordered_map<std::string, std::string> fmtCache;

    const std::string key = std::string(fmt) + "#" + std::to_string(count);
    auto& expanded = fmtCache[key];

    if (expanded.empty())
    {
        expanded.reserve(strlen(fmt) * count + count);
        for (int i = 0; i < count; ++i)
        {
            if (i > 0) expanded += " ";
            expanded += fmt;
        }
    }

    switch (count)
    {
    case 1: std::snprintf(buf, sizeof(buf), expanded.c_str(), values[0]); break;
    case 2: std::snprintf(buf, sizeof(buf), expanded.c_str(), values[0], values[1]); break;
    case 3: std::snprintf(buf, sizeof(buf), expanded.c_str(), values[0], values[1], values[2]); break;
    case 4: std::snprintf(buf, sizeof(buf), expanded.c_str(), values[0], values[1], values[2], values[3]); break;
        // extend if needed
    default: break;
    }

    return buf;
}

std::string RemoveIDCodes(std::string str)
{
    const size_t hashIdx = str.find_first_of('#');
    if (hashIdx != std::string::npos)
        return str.substr(0, hashIdx);
    return str;
}

void ImGuiUtils::FixWidthOnNext(const char* label)
{
    const float labelWidth = ImGui::CalcTextSize(label).x;
    const float innerSpacingWidth = ImGui::GetStyle().ItemInnerSpacing.x;
    ImGui::SetNextItemWidth(-(labelWidth + innerSpacingWidth));
}

void ImGuiUtils::FixWidthOnNext(const char* label, const char* field, bool& needTooltip)
{
    const float contentWidth = ImGui::GetContentRegionAvail().x;

    const float cursorX = ImGui::GetCursorScreenPos().x;
    const float labelWidth = ImGui::CalcTextSize(label).x;
    const float innerSpacingWidth = ImGui::GetStyle().ItemInnerSpacing.x;
    const float fieldWidth = ImGui::CalcTextSize(field).x;

    if (cursorX + fieldWidth + labelWidth + innerSpacingWidth > contentWidth)
    {
        needTooltip = true;
        return;
    }

    ImGui::SetNextItemWidth(-(labelWidth + innerSpacingWidth));
}

static bool s_needTooltip = false;

bool ImGuiUtils::FwInputFloat(const char* label, float* value, const float step, const float stepFast, const char* format, const ImGuiInputTextFlags flags)
{
    FixWidthOnNext(label, Format(format, *value), s_needTooltip);
    const bool result = ImGui::InputFloat(label, value, step, stepFast, format, flags);

    if (s_needTooltip && ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", RemoveIDCodes(label).c_str());

    return result;
}

bool ImGuiUtils::FwInputInt(const char* label, int* value, const int step, const int stepFast, const ImGuiInputTextFlags flags)
{
    FixWidthOnNext(label, Format("%i", *value), s_needTooltip);
    const bool result = ImGui::InputInt(label, value, step, stepFast, flags);

    if (s_needTooltip && ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", RemoveIDCodes(label).c_str());

    return result;
}

bool ImGuiUtils::FwInputUInt(const char* label, uint32_t* value, const int step, const int stepFast, const ImGuiInputTextFlags flags)
{
    FixWidthOnNext(label, Format("%i", *value), s_needTooltip);
    int sInt = static_cast<int>(*value);
    const bool result = ImGui::InputInt(label, &sInt, step, stepFast, flags);
    *value = static_cast<uint32_t>(sInt);

    if (s_needTooltip && ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", RemoveIDCodes(label).c_str());

    return result;
}

bool ImGuiUtils::FwDragFloat(const char* label, float* value, const float speed, const float min, const float max, const char* format, const ImGuiInputTextFlags flags)
{
    FixWidthOnNext(label, Format(format, *value), s_needTooltip);
    const bool result = ImGui::DragFloat(label, value, speed, min, max, format, flags);

    if (s_needTooltip && ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", RemoveIDCodes(label).c_str());

    return result;
}

bool ImGuiUtils::FwDragInt(const char* label, int* value, const float speed, const int min, const int max, const char* format, const ImGuiInputTextFlags flags)
{
    FixWidthOnNext(label, Format(format, *value), s_needTooltip);
    const bool result = ImGui::DragInt(label, value, speed, min, max, format, flags);

    if (s_needTooltip && ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", RemoveIDCodes(label).c_str());

    return result;
}

bool ImGuiUtils::FwInputFloat3(const char* label, float* value, const char* format, const ImGuiInputTextFlags flags)
{
    FixWidthOnNext(label, FormatN(format, value, 3), s_needTooltip);
    const bool result = ImGui::InputFloat3(label, value, format, flags);

    if (s_needTooltip && ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", RemoveIDCodes(label).c_str());

    return result;
}

bool ImGuiUtils::FwColorEdit3(const char* label, float* value, const ImGuiInputTextFlags flags)
{
    return ImGui::ColorEdit3(label, value, flags);
}

bool ImGuiUtils::FwColorEdit4(const char* label, float* value, const ImGuiInputTextFlags flags)
{
    return ImGui::ColorEdit4(label, value, flags);
}

bool ImGuiUtils::BeginComboWithTooltip(const char* label, const char* preview, ImGuiComboFlags flags)
{
    const bool result = ImGui::BeginCombo(label, preview, flags);

    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", RemoveIDCodes(label).c_str());

    return result;
}