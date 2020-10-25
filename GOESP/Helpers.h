#pragma once

#include <numbers>

#include "imgui/imgui.h"

struct Color;

namespace Helpers
{
    unsigned int calculateColor(Color color) noexcept;
    unsigned int calculateColor(int r, int g, int b, int a) noexcept;
    void setAlphaFactor(float newAlphaFactor) noexcept;

    constexpr auto units2meters(float units) noexcept
    {
        return units * 0.0254f;
    }

    ImWchar* getFontGlyphRanges() noexcept;

    constexpr auto deg2rad(float degrees) noexcept { return degrees * std::numbers::pi_v<float> / 180.0f; }
    constexpr auto rad2deg(float radians) noexcept { return radians * 180.0f / std::numbers::pi_v<float>; }
}