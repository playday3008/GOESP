#pragma once

#include <array>
#ifndef __APPLE__
#include <numbers>
#endif
#include <string>
#include <vector>

#include "imgui/imgui.h"

struct Color;

namespace Helpers
{
    unsigned int calculateColor(Color color, bool ignoreFlashbang = false) noexcept;
    unsigned int calculateColor(int r, int g, int b, int a) noexcept;
    void setAlphaFactor(float newAlphaFactor) noexcept;
    float getAlphaFactor() noexcept;

    constexpr auto units2meters(float units) noexcept
    {
        return units * 0.0254f;
    }

    ImWchar* getFontGlyphRanges() noexcept;
    ImWchar* getFontGlyphRangesChinese() noexcept;

#ifndef __APPLE__
    constexpr auto deg2rad(float degrees) noexcept { return degrees * (std::numbers::pi_v<float> / 180.0f); }
    constexpr auto rad2deg(float radians) noexcept { return radians * (180.0f / std::numbers::pi_v<float>); }
#else
    constexpr auto deg2rad(float degrees) noexcept { return degrees * (static_cast<float>(M_PI) / 180.0f); }
    constexpr auto rad2deg(float radians) noexcept { return radians * (180.0f / static_cast<float>(M_PI)); }
#endif

    bool decodeVFONT(std::vector<char>& buffer) noexcept;
    std::vector<char> loadBinaryFile(const std::string& path) noexcept;
    void HelpMarker(const char* desc) noexcept;
}