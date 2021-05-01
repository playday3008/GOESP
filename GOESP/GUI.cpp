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
#include "Hacks/Style.h"
#include "Helpers.h"
#include "Hooks.h"
#include "ImGuiCustom.h"
#include "Interfaces.h"
#include "Memory.h"

#include "SDK/GlobalVars.h"
#include "SDK/InputSystem.h"

static ImFont* addFontFromVFONT(const std::string& path, float size, const ImWchar* glyphRanges, bool merge, const char* name = nullptr) noexcept
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
    if (cfg.Name[0] == '\0' && name != nullptr)
        ImFormatString(cfg.Name, IM_ARRAYSIZE(cfg.Name), "%s, %dpx", name, static_cast<int>(cfg.SizePixels));
    
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
    if (!addFontFromVFONT("csgo/panorama/fonts/notosans-regular.vfont", unicodeFontSize, Helpers::getFontGlyphRanges(), false, "Noto Sans Regular"))
        io.Fonts->AddFontDefault();
    addFontFromVFONT("csgo/panorama/fonts/notosansthai-regular.vfont", unicodeFontSize, io.Fonts->GetGlyphRangesThai(), true, "Noto Sans Thai Regular");
    addFontFromVFONT("csgo/panorama/fonts/notosanskr-regular.vfont", unicodeFontSize, io.Fonts->GetGlyphRangesKorean(), true, "Noto Sans KR Regular");
    addFontFromVFONT("csgo/panorama/fonts/notosanssc-regular.vfont", unicodeFontSize, io.Fonts->GetGlyphRangesChineseFull(), true, "Noto Sans SC Regular");
    
    unicodeFont = addFontFromVFONT("csgo/panorama/fonts/notosans-bold.vfont", unicodeFontSize, Helpers::getFontGlyphRanges(), false, "Noto Sans Bold");
    addFontFromVFONT("csgo/panorama/fonts/notosansthai-bold.vfont", unicodeFontSize, io.Fonts->GetGlyphRangesThai(), true, "Noto Sans Thai Bold");
    addFontFromVFONT("csgo/panorama/fonts/notosanskr-bold.vfont", unicodeFontSize, io.Fonts->GetGlyphRangesKorean(), true, "Noto Sans KR Bold");
    addFontFromVFONT("csgo/panorama/fonts/notosanssc-bold.vfont", unicodeFontSize, io.Fonts->GetGlyphRangesChineseFull(), true, "Noto Sans SC Bold");

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
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, getTransparency());
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, { 600.f, 350.f });

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
        , nullptr, ImGuiWindowFlags_NoCollapse);

    if (open && toggleAnimationEnd < 1.0f)
        ImGui::SetWindowFocus();

    toggleAnimationEnd += ImGui::GetIO().DeltaTime / animationLength();

    if (!ImGui::BeginTabBar("##tabbar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoTooltip)) {
        ImGui::End();
        ImGui::PopStyleVar(2);
        return;
    }
    const auto buildTextSize = ImGui::CalcTextSize("Build date: " __DATE__ " " __TIME__);
    const auto unloadTextSize = ImGui::CalcTextSize("Unload");

    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - buildTextSize.x - unloadTextSize.x - 50.0f);

    ImGui::TextUnformatted("Build date: " __DATE__ " " __TIME__);

    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - unloadTextSize.x - 15.f);

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
    if (ImGui::BeginTabItem("Style")) {
        Style::drawGUI();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Configs")) {
        if (ImGui::Button("Load"))
            ImGui::OpenPopup("Load confirmation");
        if (ImGui::BeginPopup("Load confirmation")) {
            if (ImGui::Selectable("Confirm"))
                loadConfig();
            ImGui::Selectable("Cancel");
            ImGui::EndPopup();
        }
        if (ImGui::Button("Save"))
            ImGui::OpenPopup("Save confirmation");
        if (ImGui::BeginPopup("Save confirmation")) {
            if (ImGui::Selectable("Confirm"))
                saveConfig();
            ImGui::Selectable("Cancel");
            ImGui::EndPopup();
        }
        if (ImGui::Button("Open config directory")) {
            createConfigDir();
#ifdef _WIN32
            ShellExecuteW(nullptr, L"open", path.wstring().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#else
            int ret = std::system(("xdg-open " + path.string()).c_str());
#endif
        }
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Info")) {
        ImGui::Text("Original GOESP by Daniel Krupiński:");
        ImGui::SameLine();
        if (ImGui::SmallButton("Open GitHub Profile!"))
#ifdef _WIN32
            ShellExecuteW(nullptr, L"open", L"https://github.com/danielkrupinski", nullptr, nullptr, SW_SHOWNORMAL);
#else
            int ret = std::system("start https://github.com/danielkrupinski");
#endif
        ImGui::Text("GOESP BETA fork by PlayDay:");
        ImGui::SameLine();
        if (ImGui::SmallButton("Open GitHub Profile!"))
#ifdef _WIN32
            ShellExecuteW(nullptr, L"open", L"https://github.com/playday3008", nullptr, nullptr, SW_SHOWNORMAL);
#else
            int ret = std::system("start https://github.com/playday3008");
#endif
        ImGui::Text("Our Discord Server (https://discord.gg/xWEtQAn):");
        ImGui::SameLine();
        if (ImGui::SmallButton("Join!"))
#ifdef _WIN32
            ShellExecuteW(nullptr, L"open", L"https://discord.gg/xWEtQAn", nullptr, nullptr, SW_SHOWNORMAL);
#else
            int ret = std::system("start https://discord.gg/xWEtQAn");
#endif
        ImGui::Text(" ");
        ImGui::Text("Functions by:");
        ImGui::Text("Style/Font/Colors/Rendering configuration by PlayDay and ImGui Demo creators;");
        ImGui::Text("Panic Key by PlayDay;");
        ImGui::Text("Save/Load confirmation by: PlayDay");
        ImGui::Text("Audible and Spotted only in \"Ofscreen Enemies\" by: PlayDay");
#ifdef _WIN32
        ImGui::Text("BSOD button by: PlayDay");
        ImGui::SameLine();
        if (ImGui::SmallButton("BSOD"))
            ImGui::OpenPopup("Do you want to crash your Windows?");
        if (ImGui::BeginPopup("Do you want to crash your Windows?")) {
            if (ImGui::Selectable("Confirm")) {
	            if (const auto ntdll = GetModuleHandleA("ntdll.dll"); ntdll != nullptr) {
		            if (
                        const auto RtlAdjustPrivilege = GetProcAddress(ntdll, "RtlAdjustPrivilege"),
                        NtRaiseHardError = GetProcAddress(ntdll, "NtRaiseHardError");
                        RtlAdjustPrivilege && NtRaiseHardError)
                    {
                        BYTE tmp1; DWORD tmp2;
                        reinterpret_cast<void(*)(DWORD, DWORD, BOOLEAN, LPBYTE)>(RtlAdjustPrivilege)(19, true, false, &tmp1);
                        reinterpret_cast<void(*)(DWORD, DWORD, DWORD, DWORD, DWORD, LPDWORD)>(NtRaiseHardError)(0xDEADDEAD, 0, 0, 0, 6, &tmp2);
                    }
                }
            }
            ImGui::Selectable("Cancel");
            ImGui::EndPopup();
        }
        ImGui::SameLine(); Helpers::HelpMarker("WARNING: will crash your Windows (not always, but sometimes)");
        ImGui::Text("AntiDetection by: PlayDay");
#endif
        ImGui::Text("Hit Marker by: PlayDay");
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
    ImGui::End();
    ImGui::PopStyleVar(2);
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
            Style::fromJSON(j["Style"]);
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
    j["Style"] = Style::toJSON();

    removeEmptyObjects(j);

    createConfigDir();
    if (std::ofstream out{ path / "config.txt" }; out.good())
        out << std::setw(2) << j;
}

void GUI::createConfigDir() const noexcept
{
    std::error_code ec; std::filesystem::create_directory(path, ec);
}
