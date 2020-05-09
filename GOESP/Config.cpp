#include "Config.h"

#include "imgui/imgui.h"
#include "nlohmann/json.hpp"

#include <fstream>
#include <memory>
#include <ShlObj.h>
#include <Windows.h>
#include "Memory.h"

int CALLBACK fontCallback(const LOGFONTA* lpelfe, const TEXTMETRICA*, DWORD, LPARAM lParam)
{
    std::string fontName = (const char*)reinterpret_cast<const ENUMLOGFONTEXA*>(lpelfe)->elfFullName;

    if (fontName[0] == '@')
        return TRUE;

    HFONT fontHandle = CreateFontA(0, 0, 0, 0,
        FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH, fontName.c_str());

    if (fontHandle) {
        HDC hdc = CreateCompatibleDC(nullptr);

        DWORD fontData = GDI_ERROR;

        if (hdc) {
            SelectObject(hdc, fontHandle);
            // Do not use TTC fonts as we only support TTF fonts
            fontData = GetFontData(hdc, 'fctt', 0, NULL, 0);
            DeleteDC(hdc);
        }
        DeleteObject(fontHandle);

        if (fontData != GDI_ERROR)
            return TRUE;
    }
    reinterpret_cast<std::vector<std::string>*>(lParam)->push_back(fontName);
    return TRUE;
}

Config::Config(const char* folderName) noexcept
{
    if (PWSTR pathToDocuments; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &pathToDocuments))) {
        path = pathToDocuments;
        path /= folderName;
        CoTaskMemFree(pathToDocuments);
    }

    if (!std::filesystem::is_directory(path)) {
        std::filesystem::remove(path);
        std::filesystem::create_directory(path);
    }

    LOGFONTA logfont;
    logfont.lfCharSet = ANSI_CHARSET;
    logfont.lfFaceName[0] = '\0';
    logfont.lfPitchAndFamily = 0;

    EnumFontFamiliesExA(GetDC(nullptr), &logfont, fontCallback, (LPARAM)&systemFonts, 0);
    std::sort(std::next(systemFonts.begin()), systemFonts.end());
}

using json = nlohmann::basic_json<std::map, std::vector, std::string, bool, std::int64_t, std::uint64_t, float>;
using value_t = json::value_t;

template <value_t Type, typename T>
static constexpr void read(const json& j, const char* key, T& o) noexcept
{
    if (j.contains(key) && j[key].type() == Type)
        o = j[key];
}

template <value_t Type, typename T, size_t Size>
static constexpr void read(const json& j, const char* key, std::array<T, Size>& o) noexcept
{
    if (j.contains(key) && j[key].type() == Type && j[key].size() == o.size())
        o = j[key];
}

template <typename T>
static constexpr void read_number(const json& j, const char* key, T& o) noexcept
{
    if (j.contains(key) && j[key].is_number())
        o = j[key];
}

template <typename T>
static constexpr void read_map(const json& j, const char* key, T& o) noexcept
{
    if (j.contains(key) && j[key].is_object()) {
        for (auto& element : j[key].items())
            o[element.key()] = element.value();
    }
}

static void from_json(const json& j, Color& c)
{
    read<value_t::array>(j, "Color", c.color);
    read<value_t::boolean>(j, "Rainbow", c.rainbow);
    read_number(j, "Rainbow Speed", c.rainbowSpeed);
}

static void from_json(const json& j, ColorToggle& ct)
{
    from_json(j, static_cast<Color&>(ct));

    read<value_t::boolean>(j, "Enabled", ct.enabled);
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

    if (!f.name.empty())
        config->scheduleFontLoad(f.name);

    if (const auto it = std::find_if(std::cbegin(config->systemFonts), std::cend(config->systemFonts), [&f](const auto& e) { return e == f.name; }); it != std::cend(config->systemFonts))
        f.index = std::distance(std::cbegin(config->systemFonts), it);
    else
        f.index = 0;
}

static void from_json(const json& j, Snapline& s)
{
    from_json(j, static_cast<ColorToggleThickness&>(s));

    read_number(j, "Type", s.type);
}

static void from_json(const json& j, Box& b)
{
    from_json(j, static_cast<ColorToggleThicknessRounding&>(b));

    read_number(j, "Type", b.type);
    read<value_t::array>(j, "Scale", b.scale);
}

static void from_json(const json& j, Shared& s)
{
    read<value_t::boolean>(j, "Enabled", s.enabled);
    read<value_t::boolean>(j, "Use Model Bounds", s.useModelBounds);
    read<value_t::object>(j, "Font", s.font);
    read<value_t::object>(j, "Snapline", s.snapline);
    read<value_t::object>(j, "Box", s.box);
    read<value_t::object>(j, "Name", s.name);
    read<value_t::object>(j, "Text Background", s.textBackground);
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
    read<value_t::boolean>(j, "Enabled", t.enabled);
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
    read<value_t::boolean>(j, "Audible Only", p.audibleOnly);
    read<value_t::object>(j, "Skeleton", p.skeleton);
}

static void from_json(const json& j, PurchaseList& pl)
{
    read<value_t::boolean>(j, "Enabled", pl.enabled);
    read<value_t::boolean>(j, "Only During Freeze Time", pl.onlyDuringFreezeTime);
    read<value_t::boolean>(j, "Show Prices", pl.showPrices);
    read<value_t::boolean>(j, "No Title Bar", pl.noTitleBar);
    read_number(j, "Mode", pl.mode);
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
    read<value_t::boolean>(j, "Normalize Player Names", normalizePlayerNames);
    read<value_t::boolean>(j, "Bomb Zone Hint", bombZoneHint);
    read<value_t::object>(j, "Purchase List", purchaseList);
}
 
// WRITE macro requires:
// - json object named 'j'
// - object holding default values named 'dummy'
// - object to write to json named 'o'
#define WRITE(name, valueName) \
if (o.##valueName != dummy.##valueName) \
    j[name] = o.##valueName;

static void to_json(json& j, const Color& o)
{
    const Color dummy;

    WRITE("Color", color)
    WRITE("Rainbow", rainbow)
    WRITE("Rainbow Speed", rainbowSpeed)
}

static void to_json(json& j, const ColorToggle& o)
{
    j = static_cast<Color>(o);

    const ColorToggle dummy;

    WRITE("Enabled", enabled)
}

static void to_json(json& j, const ColorToggleRounding& o)
{
    j = static_cast<ColorToggle>(o);

    const ColorToggleRounding dummy;

    WRITE("Rounding", rounding)
}

static void to_json(json& j, const ColorToggleThickness& o)
{
    j = static_cast<ColorToggle>(o);

    const ColorToggleThickness dummy;

    WRITE("Thickness", thickness)
}

static void to_json(json& j, const ColorToggleThicknessRounding& o)
{
    j = static_cast<ColorToggleRounding>(o);

    const ColorToggleThicknessRounding dummy;

    WRITE("Thickness", thickness)
}

static void to_json(json& j, const Font& o)
{
    const Font dummy;

    WRITE("Name", name)
}

static void to_json(json& j, const Snapline& o)
{
    j = static_cast<ColorToggleThickness>(o);

    const Snapline dummy;

    WRITE("Type", type)
}

static void to_json(json& j, const Box& o)
{
    j = static_cast<ColorToggleThicknessRounding>(o);

    const Box dummy;

    WRITE("Type", type)
    WRITE("Scale", scale)
}

static void to_json(json& j, const Shared& o)
{
    const Shared dummy;

    WRITE("Enabled", enabled)
    WRITE("Use Model Bounds", useModelBounds)
    WRITE("Font", font)
    WRITE("Snapline", snapline)
    WRITE("Box", box)
    WRITE("Name", name)
    WRITE("Text Background", textBackground)
    WRITE("Text Cull Distance", textCullDistance)
}

static void to_json(json& j, const Player& o)
{
    j = static_cast<Shared>(o);

    const Player dummy;

    WRITE("Weapon", weapon)
    WRITE("Flash Duration", flashDuration)
    WRITE("Audible Only", audibleOnly)
    WRITE("Skeleton", skeleton)
}

static void to_json(json& j, const Weapon& o)
{
    j = static_cast<Shared>(o);

    const Weapon dummy;

    WRITE("Ammo", ammo)
}

static void to_json(json& j, const Trail& o)
{
    j = static_cast<ColorToggleThickness>(o);

    const Trail dummy;

    WRITE("Type", type)
    WRITE("Time", time)
}

static void to_json(json& j, const Trails& o)
{
    const Trails dummy;

    WRITE("Enabled", enabled)
    WRITE("Local Player", localPlayer)
    WRITE("Allies", allies)
    WRITE("Enemies", enemies)
}

static void to_json(json& j, const Projectile& o)
{
    j = static_cast<Shared>(o);

    const Projectile dummy;

    WRITE("Trails", trails)
}

static void to_json(json& j, const PurchaseList& o)
{
    const PurchaseList dummy;

    WRITE("Enabled", enabled)
    WRITE("Only During Freeze Time", onlyDuringFreezeTime)
    WRITE("Show Prices", showPrices)
    WRITE("No Title Bar", noTitleBar)
    WRITE("Mode", mode)
}

void Config::save() noexcept
{
    json j;

    for (const auto& [key, value] : allies)
        if (value != Player{})
            j["Allies"][key] = value;

    for (const auto& [key, value] : enemies)
        if (value != Player{})
            j["Enemies"][key] = value;

    for (const auto& [key, value] : weapons)
        if (value != Weapon{})
            j["Weapons"][key] = value;

    for (const auto& [key, value] : projectiles)
        if (value != Projectile{})
            j["Projectiles"][key] = value;

    for (const auto& [key, value] : lootCrates)
        if (value != Shared{})
            j["Loot Crates"][key] = value;

    for (const auto& [key, value] : otherEntities)
        if (value != Shared{})
            j["Other Entities"][key] = value;

    if (reloadProgress != ColorToggleThickness{ 5.0f })
        j["Reload Progress"] = reloadProgress;
    if (recoilCrosshair != ColorToggleThickness{})
        j["Recoil Crosshair"] = recoilCrosshair;
    if (normalizePlayerNames != true)
        j["Normalize Player Names"] = normalizePlayerNames;
    if (bombZoneHint != false)
        j["Bomb Zone Hint"] = bombZoneHint;
    if (purchaseList != PurchaseList{})
        j["Purchase List"] = purchaseList;

    if (std::ofstream out{ path / "config.txt" }; out.good())
        out << std::setw(2) << j;
}

void Config::scheduleFontLoad(const std::string& name) noexcept
{
    scheduledFonts.push_back(name);
}

bool Config::loadScheduledFonts() noexcept
{
    bool result = false;

    for (const auto& font : scheduledFonts) {
        if (font == "Default")
            continue;

        HFONT fontHandle = CreateFontA(0, 0, 0, 0,
            FW_NORMAL, FALSE, FALSE, FALSE,
            ANSI_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH, font.c_str());

        if (fontHandle) {
            HDC hdc = CreateCompatibleDC(nullptr);

            if (hdc) {
                SelectObject(hdc, fontHandle);
                auto fontDataSize = GetFontData(hdc, 0, 0, nullptr, 0);

                if (fontDataSize != GDI_ERROR) {
                    const auto fontData = std::make_unique<std::byte[]>(fontDataSize);
                    fontDataSize = GetFontData(hdc, 0, 0, fontData.get(), fontDataSize);

                    if (fontDataSize != GDI_ERROR) {
                        static constexpr ImWchar ranges[]{ 0x0020, 0xFFFF, 0 };
                        ImFontConfig cfg;
                        cfg.FontDataOwnedByAtlas = false;

                        for (int i = 8; i <= 14; i += 2) {
                            if (fonts.find(font + ' ' + std::to_string(i)) == fonts.cend()) {
                                fonts[font + ' ' + std::to_string(i)] = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(fontData.get(), fontDataSize, static_cast<float>(i), &cfg, ranges);
                                result = true;
                            }
                        }
                    }
                }
                DeleteDC(hdc);
            }
            DeleteObject(fontHandle);
        }
    }
    scheduledFonts.clear();
    return result;
}
