#include <array>
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
    
    constexpr auto unicodeFontSize = 16.0f;
    if (!addFontFromVFONT("csgo/panorama/fonts/notosans-regular.vfont", unicodeFontSize, Helpers::getFontGlyphRanges(), false))
        io.Fonts->AddFontDefault();
    addFontFromVFONT("csgo/panorama/fonts/notosansthai-regular.vfont", unicodeFontSize, io.Fonts->GetGlyphRangesThai(), true);
    addFontFromVFONT("csgo/panorama/fonts/notosanskr-regular.vfont", unicodeFontSize, io.Fonts->GetGlyphRangesKorean(), true);
    addFontFromVFONT("csgo/panorama/fonts/notosanssc-regular.vfont", unicodeFontSize, Helpers::getFontGlyphRangesChinese(), true);
    
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
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, std::clamp(open ? toggleAnimationEnd : 1.0f - toggleAnimationEnd, 0.0f, 1.0f));

    ImGui::Begin(
        "GOESP BETA for "
#ifdef _WIN32
        "Windows"
#elif __linux__
        "Linux"
#elif __APPLE__
        "macOS"
#else
#error("Unsupported platform!")
#endif
        " by PlayDay"
        , nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    if (open && toggleAnimationEnd < 1.0f)
        ImGui::SetWindowFocus();

    toggleAnimationEnd += ImGui::GetIO().DeltaTime / animationLength();

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
        if (ImGui::Button("Load"))
            loadConfig();
        if (ImGui::Button("Save"))
            saveConfig();
        if (ImGui::Button("Open config directory")) {
            createConfigDir();
#ifdef _WIN32
            int ret = std::system(("start " + path.string()).c_str());
#else
            int ret = std::system(("xdg-open " + path.string()).c_str());
#endif
        }
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Info")) {
        ImGui::Text("GOESP by danielkrupinski;");
        ImGui::Text("GOESP BETA by PlayDay (playday3008(GitHub)), (PlayDay#4049(Discord));");
        ImGui::Text("Discord by w1ldac3 (https://discord.gg/xWEtQAn);");
        ImGui::Text(" ");
        ImGui::Text("Functions by:");
        ImGui::Text("Rainbow bar by: PlayDay");
        ImGui::Text("Crashhandler support by: PlayDay and W4tev3n");

        ImGui::Text(" ");

        ImGuiStyle& style = ImGui::GetStyle();
        ImGuiIO& io = ImGui::GetIO();

        if (ImGui::CollapsingHeader("GUI Configuration by: PlayDay"))
        {
            const float MIN_SCALE = 0.3f;
            const float MAX_SCALE = 2.0f;
            ImGui::DragFloat("global scale", &io.FontGlobalScale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp); // Scale everything
            if (ImGui::TreeNode("Sizes##2"))
            {
                ImGui::Text("Main");
                ImGui::SliderFloat2("WindowPadding", reinterpret_cast<float*>(&style.WindowPadding), 0.0f, 20.0f, "%.0f");
                ImGui::SliderFloat2("FramePadding", reinterpret_cast<float*>(&style.FramePadding), 0.0f, 20.0f, "%.0f");
                ImGui::SliderFloat2("ItemSpacing", reinterpret_cast<float*>(&style.ItemSpacing), 0.0f, 20.0f, "%.0f");
                ImGui::SliderFloat2("ItemInnerSpacing", reinterpret_cast<float*>(&style.ItemInnerSpacing), 0.0f, 20.0f, "%.0f");
                ImGui::SliderFloat2("TouchExtraPadding", reinterpret_cast<float*>(&style.TouchExtraPadding), 0.0f, 10.0f, "%.0f");
                ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
                ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
                ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");
                ImGui::Text("Borders");
                ImGui::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, 1.0f, "%.0f");
                ImGui::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f, "%.0f");
                ImGui::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f, "%.0f");
                ImGui::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f, "%.0f");
                ImGui::SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, 1.0f, "%.0f");
                ImGui::Text("Rounding");
                ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");
                ImGui::SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f, "%.0f");
                ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f");
                ImGui::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f, "%.0f");
                ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f");
                ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f");
                ImGui::SliderFloat("LogSliderDeadzone", &style.LogSliderDeadzone, 0.0f, 12.0f, "%.0f");
                ImGui::SliderFloat("TabRounding", &style.TabRounding, 0.0f, 12.0f, "%.0f");
                ImGui::Text("Alignment");
                ImGui::SliderFloat2("WindowTitleAlign", reinterpret_cast<float*>(&style.WindowTitleAlign), 0.0f, 1.0f, "%.2f");
                int window_menu_button_position = style.WindowMenuButtonPosition + 1;
                if (ImGui::Combo("WindowMenuButtonPosition", static_cast<int*>(&window_menu_button_position), "None\0Left\0Right\0"))
                    style.WindowMenuButtonPosition = window_menu_button_position - 1;
                ImGui::Combo("ColorButtonPosition", static_cast<int*>(&style.ColorButtonPosition), "Left\0Right\0");
                ImGui::SliderFloat2("ButtonTextAlign", reinterpret_cast<float*>(&style.ButtonTextAlign), 0.0f, 1.0f, "%.2f");
                ImGui::SameLine(); Helpers::HelpMarker("Alignment applies when a button is larger than its text content.");
                ImGui::SliderFloat2("SelectableTextAlign", reinterpret_cast<float*>(&style.SelectableTextAlign), 0.0f, 1.0f, "%.2f");
                ImGui::SameLine(); Helpers::HelpMarker("Alignment applies when a selectable is larger than its text content.");
                ImGui::Text("Safe Area Padding");
                ImGui::SameLine(); Helpers::HelpMarker("Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured).");
                ImGui::SliderFloat2("DisplaySafeAreaPadding", reinterpret_cast<float*>(&style.DisplaySafeAreaPadding), 0.0f, 30.0f, "%.0f");
            }
            if (ImGui::TreeNode("Rendering##2"))
            {
                ImGui::Checkbox("Anti-aliased lines", &style.AntiAliasedLines);
                ImGui::SameLine(); Helpers::HelpMarker("When disabling anti-aliasing lines, you'll probably want to disable borders in your style as well.");
                ImGui::Checkbox("Anti-aliased lines use texture", &style.AntiAliasedLinesUseTex);
                ImGui::SameLine(); Helpers::HelpMarker("Faster lines using texture data. Require back-end to render with bilinear filtering (not point/nearest filtering).");
                ImGui::Checkbox("Anti-aliased fill", &style.AntiAliasedFill);
                ImGui::PushItemWidth(100);
                ImGui::DragFloat("Curve Tessellation Tolerance", &style.CurveTessellationTol, 0.02f, 0.10f, 10.0f, "%.2f");
                if (style.CurveTessellationTol < 0.10f) style.CurveTessellationTol = 0.10f;
            }
        }
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
    if (ImGui::IsKeyPressed(ImGui::GetIO().KeyMap[ImGuiKey_Insert], false)) {
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
    if (std::ifstream in{ path / "config.txt" }; in.good()) {
        if (json j = json::parse(in, nullptr, false); !j.is_discarded()) {
            ESP::fromJSON(j["ESP"]);
            Misc::fromJSON(j["Misc"]);
        }
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

    createConfigDir();
    if (std::ofstream out{ path / "config.txt" }; out.good())
        out << std::setw(2) << j;
}

void GUI::createConfigDir() const noexcept
{
    std::error_code ec; std::filesystem::create_directory(path, ec);
}
