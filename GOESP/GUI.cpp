﻿#include <array>
#include <fstream>
#include <vector>

#ifdef _WIN32
#include <ShlObj.h>
#include <Windows.h>
#else
#include <SDL2/SDL.h>
#endif

#include "imgui/imgui.h"
#include "nlohmann/json.hpp"

#include "GUI.h"
#include "Hacks/ESP.h"
#include "Hacks/Misc.h"
#include "Helpers.h"
#include "Hooks.h"
#include "ImGuiCustom.h"
#include "Interfaces.h"
#include "Memory.h"

#include "SDK/GlobalVars.h"
#include "SDK/InputSystem.h"

static ImFont* addFontFromVFONT(const std::string& path, float size, const ImWchar* glyphRanges, bool merge) noexcept
{
    auto file = Helpers::loadBinaryFile(path);
    if (!Helpers::decodeVFONT(file))
        return nullptr;
    
    ImFontConfig cfg;
    cfg.FontData = file.data();
    cfg.FontDataSize = file.size();
    cfg.FontDataOwnedByAtlas = false;
    cfg.MergeMode = merge;
    cfg.GlyphRanges = glyphRanges;
    cfg.SizePixels = size;
    cfg.RasterizerMultiply = 1.2f;

    return ImGui::GetIO().Fonts->AddFont(&cfg);
}

GUI::GUI() noexcept
{
    ImGui::StyleColorsClassic();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScrollbarSize = 13.0f;
    style.WindowTitleAlign = { 0.5f, 0.5f };
    style.Colors[ImGuiCol_WindowBg].w = 0.8f;

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.Fonts->Flags |= ImFontAtlasFlags_NoPowerOfTwoHeight;
    io.Fonts->AddFontDefault();

    constexpr auto unicodeFontSize = 16.0f;
    unicodeFont = addFontFromVFONT("csgo/panorama/fonts/notosans-bold.vfont", unicodeFontSize, Helpers::getFontGlyphRanges(), false);
    addFontFromVFONT("csgo/panorama/fonts/notosansthai-bold.vfont", unicodeFontSize, io.Fonts->GetGlyphRangesThai(), true);
    addFontFromVFONT("csgo/panorama/fonts/notosanskr-bold.vfont", unicodeFontSize, io.Fonts->GetGlyphRangesKorean(), true);
    addFontFromVFONT("csgo/panorama/fonts/notosanssc-bold.vfont", unicodeFontSize, Helpers::getFontGlyphRangesChinese(), true);

#ifdef _WIN32
    if (PWSTR pathToDocuments; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &pathToDocuments))) {
        path = pathToDocuments;
        CoTaskMemFree(pathToDocuments);
    }
#else
    if (const char* homeDir = getenv("HOME"))
        path = homeDir;
#endif
    path /= "GOESP";
}

void GUI::render() noexcept
{
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, std::clamp(open ? toggleAnimationEnd: 1.0f - toggleAnimationEnd, 0.0f, 1.0f));

    toggleAnimationEnd += ImGui::GetIO().DeltaTime / animationLength();

    ImGui::Begin(
        "GOESP for "
#ifdef _WIN32
        "Windows"
#elif __linux__
        "Linux"
#elif __APPLE__
        "macOS"
#else
#error("Unsupported platform!")
#endif
        , nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | (!open && toggleAnimationEnd > memory->globalVars->realtime ? ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove : 0));

    if (!ImGui::BeginTabBar("##tabbar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoTooltip)) {
        ImGui::End();
        ImGui::PopStyleVar();
        return;
    }

    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 350.0f);

    ImGui::TextUnformatted("Build date: " __DATE__ " " __TIME__);
    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 55.0f);

    if (ImGui::Button("Unload"))
        hooks->uninstall();

    if (ImGui::BeginTabItem("ESP")) {
        ESP::drawGUI();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Misc")) {
        Misc::drawGUI();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Configs")) {
#ifdef _WIN32
        ImGui::TextUnformatted("Config is saved as \"config.txt\" inside GOESP directory in Documents");
#elif __linux__
        ImGui::TextUnformatted("Config is saved as \"config.txt\" inside ~/GOESP directory");
#endif
        if (ImGui::Button("Load"))
            loadConfig();
        if (ImGui::Button("Save"))
            saveConfig();
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
    ImGui::End();
    ImGui::PopStyleVar();
}

ImFont* GUI::getUnicodeFont() const noexcept
{
    return unicodeFont;
}

void GUI::handleToggle() noexcept
{
#ifdef _WIN32
    if (ImGui::IsKeyPressed(VK_INSERT, false)) {
#else
    if (ImGui::IsKeyPressed(SDL_SCANCODE_INSERT, false)) {
#endif
        gui->open = !gui->open;
        if (!gui->open)
            interfaces->inputSystem->resetInputState();

        if (toggleAnimationEnd > 0.0f && toggleAnimationEnd < 1.0f)
            toggleAnimationEnd = 1.0f - toggleAnimationEnd;
        else
            toggleAnimationEnd = 0.0f;
    }
    ImGui::GetIO().MouseDrawCursor = gui->open;
}

void GUI::loadConfig() const noexcept
{
    json j;

    if (std::ifstream in{ path / "config.txt" }; in.good()) {
        in >> j;
        ESP::fromJSON(j["ESP"]);
        Misc::fromJSON(j["Misc"]);
    }
}

static void removeEmptyObjects(json& j) noexcept
{
    for (auto it = j.begin(); it != j.end();) {
        auto& val = it.value();
        if (val.is_object())
            removeEmptyObjects(val);
        if (val.empty())
            it = j.erase(it);
        else
            ++it;
    }
}

void GUI::saveConfig() const noexcept
{
    json j;

    j["ESP"] = ESP::toJSON();
    j["Misc"] = Misc::toJSON();

    removeEmptyObjects(j);

    std::error_code ec; std::filesystem::create_directory(path, ec);

    if (std::ofstream out{ path / "config.txt" }; out.good())
        out << std::setw(2) << j;
}
