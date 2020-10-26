#include <fstream>
#include <memory>

#ifdef _WIN32
#include <ShlObj.h>
#include <Windows.h>
#endif

#include "Config.h"
#include "Helpers.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "nlohmann/json.hpp"

#ifdef _WIN32
int CALLBACK fontCallback(const LOGFONTW* lpelfe, const TEXTMETRICW*, DWORD, LPARAM lParam)
{
    const wchar_t* const fontName = reinterpret_cast<const ENUMLOGFONTEXW*>(lpelfe)->elfFullName;

    if (fontName[0] == L'@')
        return TRUE;

    if (HFONT font = CreateFontW(0, 0, 0, 0,
        FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH, fontName)) {

        DWORD fontData = GDI_ERROR;

        if (HDC hdc = CreateCompatibleDC(nullptr)) {
            SelectObject(hdc, font);
            // Do not use TTC fonts as we only support TTF fonts
            fontData = GetFontData(hdc, 'fctt', 0, NULL, 0);
            DeleteDC(hdc);
        }
        DeleteObject(font);

        if (fontData == GDI_ERROR) {
            if (char buff[1024]; WideCharToMultiByte(CP_UTF8, 0, fontName, -1, buff, sizeof(buff), nullptr, nullptr))
                reinterpret_cast<std::vector<std::string>*>(lParam)->emplace_back(buff);
        }
    }
    return TRUE;
}
#endif

Config::Config(const char* folderName) noexcept
{
#ifdef _WIN32
    if (PWSTR pathToDocuments; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &pathToDocuments))) {
        path = pathToDocuments;
        CoTaskMemFree(pathToDocuments);
    }

    LOGFONTW logfont;
    logfont.lfCharSet = ANSI_CHARSET;
    logfont.lfPitchAndFamily = DEFAULT_PITCH;
    logfont.lfFaceName[0] = L'\0';

    EnumFontFamiliesExW(GetDC(nullptr), &logfont, fontCallback, (LPARAM)&systemFonts, 0);
#elif __linux__
    if (const char* homeDir = getenv("HOME"))
        path = homeDir;

    if (auto pipe = popen("fc-list :lang=en -f \"%{family[0]} %{style[0]} %{file}\\n\" | grep .ttf", "r")) {
        char* line = nullptr;
        std::size_t n = 0;
        while (getline(&line, &n, pipe) != -1) {
            auto path = strstr(line, "/");
            if (path <= line)
                continue;

            path[-1] = path[strlen(path) - 1] = '\0';
            systemFonts.emplace_back(line);
            systemFontPaths.emplace_back(path);
        }
        if (line)
            free(line);
        pclose(pipe);
    }
#endif
    path /= folderName;
    std::sort(std::next(systemFonts.begin()), systemFonts.end());
}

using json = nlohmann::basic_json<std::map, std::vector, std::string, bool, std::int64_t, std::uint64_t, float>;
using value_t = json::value_t;

template <value_t Type, typename T>
static void read(const json& j, const char* key, T& o) noexcept
{
    if (!j.contains(key))
        return;

    if (const auto& val = j[key]; val.type() == Type)
        val.get_to(o);
}

static void read(const json& j, const char* key, bool& o) noexcept
{
    if (!j.contains(key))
        return;

    if (const auto& val = j[key]; val.type() == value_t::boolean)
        val.get_to(o);
}

template <typename T, size_t Size>
static void read(const json& j, const char* key, std::array<T, Size>& o) noexcept
{
    if (!j.contains(key))
        return;

    if (const auto& val = j[key]; val.type() == value_t::array && val.size() == o.size())
        val.get_to(o);
}

template <typename T>
static void read_number(const json& j, const char* key, T& o) noexcept
{
    if (!j.contains(key))
        return;

    if (const auto& val = j[key]; val.is_number())
        val.get_to(o);
}

template <typename T>
static void read_map(const json& j, const char* key, std::unordered_map<std::string, T>& o) noexcept
{
    if (!j.contains(key))
        return;

    if (const auto& val = j[key]; val.is_object()) {
        for (auto& element : val.items())
            element.value().get_to(o[element.key()]);
    }
}

static void from_json(const json& j, Color& c)
{
    read(j, "Color", c.color);
    read(j, "Rainbow", c.rainbow);
    read_number(j, "Rainbow Speed", c.rainbowSpeed);
}

static void from_json(const json& j, ColorToggle& ct)
{
    from_json(j, static_cast<Color&>(ct));

    read(j, "Enabled", ct.enabled);
}

static void from_json(const json& j, ColorToggleRounding& ctr)
{
    from_json(j, static_cast<ColorToggle&>(ctr));

    read_number(j, "Rounding", ctr.rounding);
}

static void from_json(const json& j, ColorToggleThickness& ctt)
{
    from_json(j, static_cast<ColorToggle&>(ctt));

    read_number(j, "Thickness", ctt.thickness);
}

static void from_json(const json& j, ColorToggleThicknessRounding& cttr)
{
    from_json(j, static_cast<ColorToggleRounding&>(cttr));

    read_number(j, "Thickness", cttr.thickness);
}

static void from_json(const json& j, Font& f)
{
    read<value_t::string>(j, "Name", f.name); 

    if (const auto it = std::find_if(config->getSystemFonts().begin(), config->getSystemFonts().end(), [&f](const auto& e) { return e == f.name; }); it != config->getSystemFonts().end()) {
        f.index = std::distance(config->getSystemFonts().begin(), it);
        config->scheduleFontLoad(f.index);
    } else {
        f.index = 0;
    }
}

static void from_json(const json& j, Snapline& s)
{
    from_json(j, static_cast<ColorToggleThickness&>(s));

    read_number(j, "Type", s.type);
}

static void from_json(const json& j, Box& b)
{
    from_json(j, static_cast<ColorToggleRounding&>(b));

    read_number(j, "Type", b.type);
    read(j, "Scale", b.scale);
    read<value_t::object>(j, "Fill", b.fill);
}

static void from_json(const json& j, Shared& s)
{
    read(j, "Enabled", s.enabled);
    read<value_t::object>(j, "Font", s.font);
    read<value_t::object>(j, "Snapline", s.snapline);
    read<value_t::object>(j, "Box", s.box);
    read<value_t::object>(j, "Name", s.name);
    read_number(j, "Text Cull Distance", s.textCullDistance);
}

static void from_json(const json& j, Weapon& w)
{
    from_json(j, static_cast<Shared&>(w));

    read<value_t::object>(j, "Ammo", w.ammo);
}

static void from_json(const json& j, Trail& t)
{
    from_json(j, static_cast<ColorToggleThickness&>(t));

    read_number(j, "Type", t.type);
    read_number(j, "Time", t.time);
}

static void from_json(const json& j, Trails& t)
{
    read(j, "Enabled", t.enabled);
    read<value_t::object>(j, "Local Player", t.localPlayer);
    read<value_t::object>(j, "Allies", t.allies);
    read<value_t::object>(j, "Enemies", t.enemies);
}

static void from_json(const json& j, Projectile& p)
{
    from_json(j, static_cast<Shared&>(p));

    read<value_t::object>(j, "Trails", p.trails);
}

static void from_json(const json& j, Player& p)
{
    from_json(j, static_cast<Shared&>(p));

    read<value_t::object>(j, "Weapon", p.weapon);
    read<value_t::object>(j, "Flash Duration", p.flashDuration);
    read(j, "Audible Only", p.audibleOnly);
    read(j, "Spotted Only", p.spottedOnly);
    read<value_t::object>(j, "Skeleton", p.skeleton);
    read<value_t::object>(j, "Head Box", p.headBox);
    read(j, "Health Bar", p.healthBar);
}

static void from_json(const json& j, ImVec2& v)
{
    read_number(j, "X", v.x);
    read_number(j, "Y", v.y);
}

static void from_json(const json& j, PurchaseList& pl)
{
    read(j, "Enabled", pl.enabled);
    read(j, "Only During Freeze Time", pl.onlyDuringFreezeTime);
    read(j, "Show Prices", pl.showPrices);
    read(j, "No Title Bar", pl.noTitleBar);
    read_number(j, "Mode", pl.mode);
    read<value_t::object>(j, "Pos", pl.pos);
    read<value_t::object>(j, "Size", pl.size);
}

static void from_json(const json& j, ObserverList& ol)
{
    read(j, "Enabled", ol.enabled);
    read(j, "No Title Bar", ol.noTitleBar);
    read<value_t::object>(j, "Pos", ol.pos);
    read<value_t::object>(j, "Size", ol.size);
}

static void from_json(const json& j, OverlayWindow& o)
{
    read(j, "Enabled", o.enabled);
    read<value_t::object>(j, "Pos", o.pos);
}

static void from_json(const json& j, OffscreenEnemies& o)
{
    read(j, "Enabled", o.enabled);
    read<value_t::object>(j, "Color", o.color);
}

void Config::load() noexcept
{
    json j;

    if (std::ifstream in{ path / "config.txt" }; in.good())
        in >> j;
    else
        return;

    read_map(j, "Allies", allies);
    read_map(j, "Enemies", enemies);
    read_map(j, "Weapons", weapons);
    read_map(j, "Projectiles", projectiles);
    read_map(j, "Loot Crates", lootCrates);
    read_map(j, "Other Entities", otherEntities);

    read<value_t::object>(j, "Reload Progress", reloadProgress);
    read<value_t::object>(j, "Recoil Crosshair", recoilCrosshair);
    read<value_t::object>(j, "Noscope Crosshair", noscopeCrosshair);
    read<value_t::object>(j, "Purchase List", purchaseList);
    read<value_t::object>(j, "Observer List", observerList);
    read(j, "Ignore Flashbang", ignoreFlashbang);
    read<value_t::object>(j, "FPS Counter", fpsCounter);
    read<value_t::object>(j, "Offscreen Enemies", offscreenEnemies);

    read<value_t::object>(j, "Rainbow Bar", rainbowBar);
    read(j, "Rainbow Up", rainbowUp);
    read(j, "Rainbow Bottom", rainbowBottom);
    read(j, "Rainbow Left", rainbowLeft);
    read(j, "Rainbow Right", rainbowRight);
    read_number(j, "Rainbow Scale", rainbowScale);
    read(j, "Rainbow Pulse", rainbowPulse);
    read_number(j, "Rainbow Pulse Speed", rainbowPulseSpeed);

    read<value_t::object>(j, "Watermark", watermark);
    read(j, "Watermark Nickname", watermarkNickname);
    read(j, "Watermark Username", watermarkUsername);
    read(j, "Watermark FPS", watermarkFPS);
    read(j, "Watermark Ping", watermarkPing);
    read(j, "Watermark Tickrate", watermarkTickrate);
    read(j, "Watermark Velocity", watermarkVelocity);
    read(j, "Watermark Time", watermarkTime);
    read<value_t::object>(j, "Watermark Pos", watermarkPos);
    read_number(j, "Watermark Scale", watermarkScale);

    read<value_t::object>(j, "Hit Marker", hitMarker);
    read_number(j, "Hit Marker Length", hitMarkerLength);
    read_number(j, "Hit Marker Time", hitMarkerTime);

    read<value_t::object>(j, "Hit marker damage indicator", hitMarkerDamageIndicator);
    read(j, "Hit marker damage indicator Customize", hitMarkerDamageIndicatorCustomize);
    read_number(j, "Hit marker damage indicator Dist", hitMarkerDamageIndicatorDist);
    read_number(j, "Hit marker damage indicator Ratio", hitMarkerDamageIndicatorRatio);

    read<value_t::object>(j, "Bomb timer", bombTimer);

    read_number(j, "Menu Color", menuColors);
    if (j.contains("Colors") && j["Colors"].is_object()) {
        const auto& colors = j["Colors"];

        ImGuiStyle& style = ImGui::GetStyle();

        for (int i = 0; i < ImGuiCol_COUNT; i++) {
            if (const char* name = ImGui::GetStyleColorName(i); colors.contains(name)) {
                std::array<float, 4> temp;
                read(colors, name, temp);
                style.Colors[i].x = temp[0];
                style.Colors[i].y = temp[1];
                style.Colors[i].z = temp[2];
                style.Colors[i].w = temp[3];
            }
        }
    }
    read<value_t::object>(j, "Menu color Custom (Easy) BackGroundColor", customEasy.BackGroundColor);
    read<value_t::object>(j, "Menu color Custom (Easy) HighlightColor", customEasy.HighlightColor);
    read<value_t::object>(j, "Menu color Custom (Easy) MainAccentColor", customEasy.MainAccentColor);
    read<value_t::object>(j, "Menu color Custom (Easy) MainColor", customEasy.MainColor);
    read<value_t::object>(j, "Menu color Custom (Easy) TextColor", customEasy.TextColor);

    // Load GUI Configuration
    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO();
    // Font scale
    read_number(j, "global scale", io.FontGlobalScale);
    // Main
    read<value_t::object>(j, "WindowPadding", style.WindowPadding);
    read<value_t::object>(j, "FramePadding", style.FramePadding);
    read<value_t::object>(j, "ItemSpacing", style.ItemSpacing);
    read<value_t::object>(j, "ItemInnerSpacing", style.ItemInnerSpacing);
    read<value_t::object>(j, "TouchExtraPadding", style.TouchExtraPadding);
    read_number(j, "IndentSpacing", style.IndentSpacing);
    read_number(j, "ScrollbarSize", style.ScrollbarSize);
    read_number(j, "GrabMinSize", style.GrabMinSize);
    // Borders
    read_number(j, "WindowBorderSize", style.WindowBorderSize);
    read_number(j, "ChildBorderSize", style.ChildBorderSize);
    read_number(j, "PopupBorderSize", style.PopupBorderSize);
    read_number(j, "FrameBorderSize", style.FrameBorderSize);
    read_number(j, "TabBorderSize", style.TabBorderSize);
    // Rounding
    read_number(j, "WindowRounding", style.WindowRounding);
    read_number(j, "ChildRounding", style.ChildRounding);
    read_number(j, "FrameRounding", style.FrameRounding);
    read_number(j, "PopupRounding", style.PopupRounding);
    read_number(j, "ScrollbarRounding", style.ScrollbarRounding);
    read_number(j, "GrabRounding", style.GrabRounding);
    read_number(j, "LogSliderDeadzone", style.LogSliderDeadzone);
    read_number(j, "TabRounding", style.TabRounding);
    // Alignment
    read<value_t::object>(j, "WindowTitleAlign", style.WindowTitleAlign);
    read_number(j, "WindowMenuButtonPosition", style.WindowMenuButtonPosition);
    read_number(j, "ColorButtonPosition", style.ColorButtonPosition);
    read<value_t::object>(j, "ButtonTextAlign", style.ButtonTextAlign);
    read<value_t::object>(j, "SelectableTextAlign", style.SelectableTextAlign);
    read<value_t::object>(j, "DisplaySafeAreaPadding", style.DisplaySafeAreaPadding);
    // Renering
    read(j, "Anti-aliased lines", style.AntiAliasedLines);
    read(j, "Anti-aliased lines use texture", style.AntiAliasedLinesUseTex);
    read(j, "Anti-aliased fill", style.AntiAliasedFill);
    read_number(j, "Curve Tessellation Tolerance", style.CurveTessellationTol);
}

// WRITE macro requires:
// - json object named 'j'
// - object holding default values named 'dummy'
// - object to write to json named 'o'
#define WRITE(name, valueName) \
if (!(o.valueName == dummy.valueName)) \
    j[name] = o.valueName;

static void to_json(json& j, const Color& o, const Color& dummy = {})
{
    WRITE("Color", color)
    WRITE("Rainbow", rainbow)
    WRITE("Rainbow Speed", rainbowSpeed)
}

static void to_json(json& j, const ColorToggle& o, const ColorToggle& dummy = {})
{
    to_json(j, static_cast<const Color&>(o), dummy);
    WRITE("Enabled", enabled)
}

static void to_json(json& j, const ColorToggleRounding& o, const ColorToggleRounding& dummy = {})
{
    to_json(j, static_cast<const ColorToggle&>(o), dummy);
    WRITE("Rounding", rounding)
}

static void to_json(json& j, const ColorToggleThickness& o, const ColorToggleThickness& dummy = {})
{
    to_json(j, static_cast<const ColorToggle&>(o), dummy);
    WRITE("Thickness", thickness)
}

static void to_json(json& j, const ColorToggleThicknessRounding& o, const ColorToggleThicknessRounding& dummy = {})
{
    to_json(j, static_cast<const ColorToggleRounding&>(o), dummy);
    WRITE("Thickness", thickness)
}

static void to_json(json& j, const Font& o, const Font& dummy = {})
{
    WRITE("Name", name)
}

static void to_json(json& j, const Snapline& o, const Snapline& dummy = {})
{
    to_json(j, static_cast<const ColorToggleThickness&>(o), dummy);
    WRITE("Type", type)
}

static void to_json(json& j, const Box& o, const Box& dummy = {})
{
    to_json(j, static_cast<const ColorToggleRounding&>(o), dummy);
    WRITE("Type", type)
    WRITE("Scale", scale)
    to_json(j["Fill"], o.fill, dummy.fill);
}

static void to_json(json& j, const Shared& o, const Shared& dummy = {})
{
    WRITE("Enabled", enabled)
    to_json(j["Font"], o.font, dummy.font);
    to_json(j["Snapline"], o.snapline, dummy.snapline);
    to_json(j["Box"], o.box, dummy.box);
    to_json(j["Name"], o.name, dummy.name);
    WRITE("Text Cull Distance", textCullDistance)
}

static void to_json(json& j, const Player& o, const Player& dummy = {})
{
    to_json(j, static_cast<const Shared&>(o), dummy);
    to_json(j["Weapon"], o.weapon, dummy.weapon);
    to_json(j["Flash Duration"], o.flashDuration, dummy.flashDuration);
    WRITE("Audible Only", audibleOnly)
    WRITE("Spotted Only", spottedOnly)
    to_json(j["Skeleton"], o.skeleton, dummy.skeleton);
    to_json(j["Head Box"], o.headBox, dummy.headBox);
    WRITE("Health Bar", healthBar)
}

static void to_json(json& j, const Weapon& o, const Weapon& dummy = {})
{
    to_json(j, static_cast<const Shared&>(o), dummy);
    to_json(j["Ammo"], o.ammo, dummy.ammo);
}

static void to_json(json& j, const Trail& o, const Trail& dummy = {})
{
    to_json(j, static_cast<const ColorToggleThickness&>(o), dummy);
    WRITE("Type", type)
    WRITE("Time", time)
}

static void to_json(json& j, const Trails& o, const Trails& dummy = {})
{
    WRITE("Enabled", enabled)
    to_json(j["Local Player"], o.localPlayer, dummy.localPlayer);
    to_json(j["Allies"], o.allies, dummy.allies);
    to_json(j["Enemies"], o.enemies, dummy.enemies);
}

static void to_json(json& j, const Projectile& o, const Projectile& dummy = {})
{
    to_json(j, static_cast<const Shared&>(o), dummy);
    to_json(j["Trails"], o.trails, dummy.trails);
}

static void to_json(json& j, const ImVec2& o, const ImVec2& dummy = {})
{
    WRITE("X", x)
    WRITE("Y", y)
}

static void to_json(json& j, const PurchaseList& o, const PurchaseList& dummy = {})
{
    WRITE("Enabled", enabled)
    WRITE("Only During Freeze Time", onlyDuringFreezeTime)
    WRITE("Show Prices", showPrices)
    WRITE("No Title Bar", noTitleBar)
    WRITE("Mode", mode)

    if (const auto window = ImGui::FindWindowByName("Purchases")) {
        j["Pos"] = window->Pos;
        j["Size"] = window->SizeFull;
    }
}

static void to_json(json& j, const ObserverList& o, const ObserverList& dummy = {})
{
    WRITE("Enabled", enabled)
    WRITE("No Title Bar", noTitleBar)

    if (const auto window = ImGui::FindWindowByName("Observer List")) {
        j["Pos"] = window->Pos;
        j["Size"] = window->SizeFull;
    }
}

static void to_json(json& j, const OverlayWindow& o, const OverlayWindow& dummy = {})
{
    WRITE("Enabled", enabled)

    if (const auto window = ImGui::FindWindowByName(o.name))
        j["Pos"] = window->Pos;
}

static void to_json(json& j, const OffscreenEnemies& o, const OffscreenEnemies& dummy = {})
{
    WRITE("Enabled", enabled)
    to_json(j["Color"], o.color, dummy.color);
}

static void to_json(json& j, const ImVec4& o)
{
    j[0] = o.x;
    j[1] = o.y;
    j[2] = o.z;
    j[3] = o.w;
}

void removeEmptyObjects(json& j) noexcept
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

void Config::save() noexcept
{
    json j;

    j["Allies"] = allies;
    j["Enemies"] = enemies;
    j["Weapons"] = weapons;
    j["Projectiles"] = projectiles;
    j["Loot Crates"] = lootCrates;
    j["Other Entities"] = otherEntities;

    to_json(j["Reload Progress"], reloadProgress, ColorToggleThickness{ 5.0f });

    if (ignoreFlashbang)
        j["Ignore Flashbang"] = ignoreFlashbang;

    j["Recoil Crosshair"] = recoilCrosshair;
    j["Noscope Crosshair"] = noscopeCrosshair;
    j["Purchase List"] = purchaseList;
    j["Observer List"] = observerList;
    j["FPS Counter"] = fpsCounter;
    j["Offscreen Enemies"] = offscreenEnemies;

    j["Rainbow Bar"] = rainbowBar;
    j["Rainbow Up"] = rainbowUp;
    j["Rainbow Bottom"] = rainbowBottom;
    j["Rainbow Left"] = rainbowLeft;
    j["Rainbow Right"] = rainbowRight;
    j["Rainbow Scale"] = rainbowScale;
    j["Rainbow Pulse"] = rainbowPulse;
    j["Rainbow Pulse Speed"] = rainbowPulseSpeed;

    j["Watermark"] = watermark;
    j["Watermark Nickname"] = watermarkNickname;
    j["Watermark Username"] = watermarkUsername;
    j["Watermark FPS"] = watermarkFPS;
    j["Watermark Ping"] = watermarkPing;
    j["Watermark Tickrate"] = watermarkTickrate;
    j["Watermark Velocity"] = watermarkVelocity;
    j["Watermark Time"] = watermarkTime;
    j["Watermark Pos"] = watermarkPos;
    j["Watermark Scale"] = watermarkScale;

    j["Hit Marker"] = hitMarker;
    j["Hit Marker Length"] = hitMarkerLength;
    j["Hit Marker Time"] = hitMarkerTime;

    j["Hit marker damage indicator"] = hitMarkerDamageIndicator;
    j["Hit marker damage indicator Customize"] = hitMarkerDamageIndicatorCustomize;
    j["Hit marker damage indicator Dist"] = hitMarkerDamageIndicatorDist;
    j["Hit marker damage indicator Ratio"] = hitMarkerDamageIndicatorRatio;

    j["Bomb timer"] = bombTimer;

    j["Menu Color"] = menuColors;
    auto& colors = j["Colors"];
    ImGuiStyle& style = ImGui::GetStyle();

    for (int i = 0; i < ImGuiCol_COUNT; i++)
        colors[ImGui::GetStyleColorName(i)] = style.Colors[i];
    j["Menu color Custom (Easy) BackGroundColor"] = customEasy.BackGroundColor;
    j["Menu color Custom (Easy) HighlightColor"] = customEasy.HighlightColor;
    j["Menu color Custom (Easy) MainAccentColor"] = customEasy.MainAccentColor;
    j["Menu color Custom (Easy) MainColor"] = customEasy.MainColor;
    j["Menu color Custom (Easy) TextColor"] = customEasy.TextColor;

    // Save GUI Configuration
    ImGuiIO& io = ImGui::GetIO();
    // Font scale
    j["global scale"] = io.FontGlobalScale;
    // Main
    j["WindowPadding"] = style.WindowPadding;
    j["FramePadding"] = style.FramePadding;
    j["ItemSpacing"] = style.ItemSpacing;
    j["ItemInnerSpacing"] = style.ItemInnerSpacing;
    j["TouchExtraPadding"] = style.TouchExtraPadding;
    j["IndentSpacing"] = style.IndentSpacing;
    j["ScrollbarSize"] = style.ScrollbarSize;
    j["GrabMinSize"] = style.GrabMinSize;
    // Borders
    j["WindowBorderSize"] = style.WindowBorderSize;
    j["ChildBorderSize"] = style.ChildBorderSize;
    j["PopupBorderSize"] = style.PopupBorderSize;
    j["FrameBorderSize"] = style.FrameBorderSize;
    j["TabBorderSize"] = style.TabBorderSize;
    // Rounding
    j["WindowRounding"] = style.WindowRounding;
    j["ChildRounding"] = style.ChildRounding;
    j["FrameRounding"] = style.FrameRounding;
    j["PopupRounding"] = style.PopupRounding;
    j["ScrollbarRounding"] = style.ScrollbarRounding;
    j["GrabRounding"] = style.GrabRounding;
    j["LogSliderDeadzone"] = style.LogSliderDeadzone;
    j["TabRounding"] = style.TabRounding;
    // Alignment
    j["WindowTitleAlign"] = style.WindowTitleAlign;
    j["WindowMenuButtonPosition"] = style.WindowMenuButtonPosition;
    j["ColorButtonPosition"] = style.ColorButtonPosition;
    j["ButtonTextAlign"] = style.ButtonTextAlign;
    j["SelectableTextAlign"] = style.SelectableTextAlign;
    j["DisplaySafeAreaPadding"] = style.DisplaySafeAreaPadding;
    // Renering
    j["Anti-aliased lines"] = style.AntiAliasedLines;
    j["Anti-aliased lines use texture"] = style.AntiAliasedLinesUseTex;
    j["Anti-aliased fill"] = style.AntiAliasedFill;
    j["Curve Tessellation Tolerance"] = style.CurveTessellationTol;

    removeEmptyObjects(j);

    std::error_code ec; std::filesystem::create_directory(path, ec);

    if (std::ofstream out{ path / "config.txt" }; out.good())
        out << std::setw(2) << j;
}

void Config::scheduleFontLoad(std::size_t index) noexcept
{
    scheduledFonts.push_back(index);
}

static auto getFontData(const std::string& fontName) noexcept
{
#ifdef _WIN32
    HFONT font = CreateFontA(0, 0, 0, 0,
        FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH, fontName.c_str());

    std::unique_ptr<std::byte[]> data;
    DWORD dataSize = GDI_ERROR;

    if (font) {
        HDC hdc = CreateCompatibleDC(nullptr);

        if (hdc) {
            SelectObject(hdc, font);
            dataSize = GetFontData(hdc, 0, 0, nullptr, 0);

            if (dataSize != GDI_ERROR) {
                data = std::make_unique<std::byte[]>(dataSize);
                dataSize = GetFontData(hdc, 0, 0, data.get(), dataSize);

                if (dataSize == GDI_ERROR)
                    data.reset();
            }
            DeleteDC(hdc);
        }
        DeleteObject(font);
    }
    return std::make_pair(std::move(data), dataSize);
#else
    std::size_t dataSize = (std::size_t)-1;
    auto data = (std::byte*)ImFileLoadToMemory(fontName.c_str(), "rb", &dataSize);
    return std::make_pair(std::unique_ptr<std::byte[]>{ data }, dataSize);
#endif

}

bool Config::loadScheduledFonts() noexcept
{
    bool result = false;

    for (const auto fontIndex : scheduledFonts) {
        const auto& fontName = systemFonts[fontIndex];

        if (fonts.contains(fontName))
            continue;

        ImFontConfig cfg;
        Font newFont;

        if (fontName == "Default") {
            cfg.SizePixels = 13.0f;
            newFont.big = ImGui::GetIO().Fonts->AddFontDefault(&cfg);

            cfg.SizePixels = 10.0f;
            newFont.medium = ImGui::GetIO().Fonts->AddFontDefault(&cfg);

            cfg.SizePixels = 8.0f;
            newFont.tiny = ImGui::GetIO().Fonts->AddFontDefault(&cfg);

            fonts.emplace(fontName, newFont);
        } else {
#ifdef _WIN32
            const auto& fontPath = fontName;
#else
            const auto& fontPath = systemFontPaths[fontIndex];
#endif
            const auto [fontData, fontDataSize] = getFontData(fontPath);
            if (fontDataSize == -1)
                continue;

            cfg.FontDataOwnedByAtlas = false;
            const auto ranges = Helpers::getFontGlyphRanges();

            newFont.tiny = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(fontData.get(), fontDataSize, 8.0f, &cfg, ranges);
            newFont.medium = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(fontData.get(), fontDataSize, 10.0f, &cfg, ranges);
            newFont.big = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(fontData.get(), fontDataSize, 13.0f, &cfg, ranges);
            fonts.emplace(fontName, newFont);
        }
        result = true;
    }
    scheduledFonts.clear();
    return result;
}
