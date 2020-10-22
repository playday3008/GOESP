#pragma once

#include <array>
#include <filesystem>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "ConfigStructs.h"

struct ImFont;

class Config {
public:
    explicit Config(const char* folderName) noexcept;
    void load() noexcept;
    void save() noexcept;

    std::unordered_map<std::string, Player> allies;
    std::unordered_map<std::string, Player> enemies;
    std::unordered_map<std::string, Weapon> weapons;
    std::unordered_map<std::string, Projectile> projectiles;
    std::unordered_map<std::string, Shared> lootCrates;
    std::unordered_map<std::string, Shared> otherEntities;

    ColorToggleThickness reloadProgress{ 5.0f };
    ColorToggleThickness recoilCrosshair;
    ColorToggleThickness noscopeCrosshair;
    PurchaseList purchaseList;
    ObserverList observerList;
    bool ignoreFlashbang = false;
    OverlayWindow fpsCounter{ "FPS Counter" };

    ColorToggleThickness rainbowBar{ 3.0f };
    bool rainbowUp{ true };
    bool rainbowBottom{ false };
    bool rainbowLeft{ false };
    bool rainbowRight{ false };
    float rainbowScale{ 0.125f };
    bool rainbowPulse{ false };
    float rainbowPulseSpeed{ 1.0f };

    ColorToggle watermark;
    bool watermarkNickname{ true };
    bool watermarkUsername{ true };
    bool watermarkFPS{ true };
    bool watermarkPing{ true };
    bool watermarkTickrate{ true };
    bool watermarkTime{ true };
    ImVec2 watermarkPos{ 0.f,0.f };
    float watermarkScale{ 1.0f };

    ColorToggleThickness hitMarker;
    float hitMarkerLength{ 10.f };
    float hitMarkerTime{ 0.6f };

    int menuColors{ 0 };
    struct StyleCustomEasy {
        Color BackGroundColor;
        Color TextColor;
        Color MainColor;
        Color MainAccentColor;
        Color HighlightColor;
    } customEasy;

    struct Font {
        ImFont* tiny;
        ImFont* medium;
        ImFont* big;
    };

    void scheduleFontLoad(std::size_t index) noexcept;
    bool loadScheduledFonts() noexcept;
    const auto& getSystemFonts() noexcept { return systemFonts; }
    const auto& getFonts() noexcept { return fonts; }
private:
    std::vector<std::size_t> scheduledFonts{ 0 };
    std::vector<std::string> systemFonts{ "Default" };
    std::unordered_map<std::string, Font> fonts;
#ifndef _WIN32
    std::vector<std::string> systemFontPaths{ "" };
#endif
    std::filesystem::path path;
};

inline std::unique_ptr<Config> config;
