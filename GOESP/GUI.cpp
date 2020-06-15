#include "GUI.h"

#include "imgui/imgui.h"

#include "Config.h"
#include "Hooks.h"
#include "ImGuiCustom.h"

#include <array>
#include <vector>

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
}

void GUI::render() noexcept
{
    if (!open)
        return;

    ImGui::Begin("GOESP", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse);

    if (!ImGui::BeginTabBar("##tabbar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoTooltip)) {
        ImGui::End();
        return;
    }

    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 350.0f);

    ImGui::TextUnformatted("Build date: " __DATE__ " " __TIME__);
    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 55.0f);

    if (ImGui::Button("Unload"))
        hooks->uninstall();

    if (ImGui::BeginTabItem("ESP")) {
        drawESPTab();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Misc")) {
        ImGuiCustom::colorPicker("Reload Progress", config->reloadProgress);
        ImGuiCustom::colorPicker("Recoil Crosshair", config->recoilCrosshair);
        ImGuiCustom::colorPicker("Noscope Crosshair", config->noscopeCrosshair);
        ImGui::Checkbox("Purchase List", &config->purchaseList.enabled);
        ImGui::SameLine();

        ImGui::PushID("Purchase List");
        if (ImGui::Button("..."))
            ImGui::OpenPopup("");

        if (ImGui::BeginPopup("")) {
            ImGui::SetNextItemWidth(75.0f);
            ImGui::Combo("Mode", &config->purchaseList.mode, "Details\0Summary\0");
            ImGui::Checkbox("Only During Freeze Time", &config->purchaseList.onlyDuringFreezeTime);
            ImGui::Checkbox("Show Prices", &config->purchaseList.showPrices);
            ImGui::Checkbox("No Title Bar", &config->purchaseList.noTitleBar);
            ImGui::EndPopup();
        }
        ImGui::PopID();

        ImGui::PushID("Observer List");
        ImGui::Checkbox("Observer List", &config->observerList.enabled);
        ImGui::SameLine();

        if (ImGui::Button("..."))
            ImGui::OpenPopup("");

        if (ImGui::BeginPopup("")) {
            ImGui::Checkbox("No Title Bar", &config->observerList.noTitleBar);
            ImGui::EndPopup();
        }
        ImGui::PopID();

        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Configs")) {
        ImGui::TextUnformatted("Config is saved as \"config.txt\" inside GOESP directory in Documents");
        if (ImGui::Button("Load"))
            config->load();
        if (ImGui::Button("Save"))
            config->save();
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
    ImGui::End();
}

void GUI::drawESPTab() noexcept
{
    static std::size_t currentCategory;
    static auto currentItem = "All";

    constexpr auto getConfigShared = [](std::size_t category, const char* item) noexcept -> Shared& {
        switch (category) {
        case 0: default: return config->enemies[item];
        case 1: return config->allies[item];
        case 2: return config->weapons[item];
        case 3: return config->projectiles[item];
        case 4: return config->lootCrates[item];
        case 5: return config->otherEntities[item];
        }
    };

    constexpr auto getConfigPlayer = [](std::size_t category, const char* item) noexcept -> Player& {
        switch (category) {
        case 0: default: return config->enemies[item];
        case 1: return config->allies[item];
        }
    };

    if (ImGui::ListBoxHeader("##list", { 170.0f, 300.0f })) {
        constexpr std::array categories{ "Enemies", "Allies", "Weapons", "Projectiles", "Loot Crates", "Other Entities" };

        for (std::size_t i = 0; i < categories.size(); ++i) {
            if (ImGui::Selectable(categories[i], currentCategory == i && std::string_view{ currentItem } == "All")) {
                currentCategory = i;
                currentItem = "All";
            }

            if (ImGui::BeginDragDropSource()) {
                switch (i) {
                case 0: case 1: ImGui::SetDragDropPayload("Player", &getConfigPlayer(i, "All"), sizeof(Player), ImGuiCond_Once); break;
                case 2: ImGui::SetDragDropPayload("Weapon", &config->weapons["All"], sizeof(Weapon), ImGuiCond_Once); break;
                case 3: ImGui::SetDragDropPayload("Projectile", &config->projectiles["All"], sizeof(Projectile), ImGuiCond_Once); break;
                default: ImGui::SetDragDropPayload("Entity", &getConfigShared(i, "All"), sizeof(Shared), ImGuiCond_Once); break;
                }
                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Player")) {
                    const auto& data = *(Player*)payload->Data;

                    switch (i) {
                    case 0: case 1: getConfigPlayer(i, "All") = data; break;
                    case 2: config->weapons["All"] = data; break;
                    case 3: config->projectiles["All"] = data; break;
                    default: getConfigShared(i, "All") = data; break;
                    }
                }

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Weapon")) {
                    const auto& data = *(Weapon*)payload->Data;

                    switch (i) {
                    case 0: case 1: getConfigPlayer(i, "All") = data; break;
                    case 2: config->weapons["All"] = data; break;
                    case 3: config->projectiles["All"] = data; break;
                    default: getConfigShared(i, "All") = data; break;
                    }
                }

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Projectile")) {
                    const auto& data = *(Projectile*)payload->Data;

                    switch (i) {
                    case 0: case 1: getConfigPlayer(i, "All") = data; break;
                    case 2: config->weapons["All"] = data; break;
                    case 3: config->projectiles["All"] = data; break;
                    default: getConfigShared(i, "All") = data; break;
                    }
                }

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity")) {
                    const auto& data = *(Shared*)payload->Data;

                    switch (i) {
                    case 0: case 1: getConfigPlayer(i, "All") = data; break;
                    case 2: config->weapons["All"] = data; break;
                    case 3: config->projectiles["All"] = data; break;
                    default: getConfigShared(i, "All") = data; break;
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::PushID(i);
            ImGui::Indent();

            const auto items = [](std::size_t category) noexcept -> std::vector<const char*> {
                switch (category) {
                case 0:
                case 1: return { "Visible", "Occluded" };
                case 2: return { "Pistols", "SMGs", "Rifles", "Sniper Rifles", "Shotguns", "Machineguns", "Grenades", "Melee", "Other" };
                case 3: return { "Flashbang", "HE Grenade", "Breach Charge", "Bump Mine", "Decoy Grenade", "Molotov", "TA Grenade", "Smoke Grenade", "Snowball" };
                case 4: return { "Pistol Case", "Light Case", "Heavy Case", "Explosive Case", "Tools Case", "Cash Dufflebag" };
                case 5: return { "Defuse Kit", "Chicken", "Planted C4", "Hostage", "Sentry", "Cash", "Ammo Box", "Radar Jammer", "Snowball Pile" };
                default: return { };
                }
            }(i);

            const auto categoryEnabled = getConfigShared(i, "All").enabled;

            for (std::size_t j = 0; j < items.size(); ++j) {
                static bool selectedSubItem;
                if (!categoryEnabled || getConfigShared(i, items[j]).enabled) {
                    if (ImGui::Selectable(items[j], currentCategory == i && !selectedSubItem && std::string_view{ currentItem } == items[j])) {
                        currentCategory = i;
                        currentItem = items[j];
                        selectedSubItem = false;
                    }

                    if (ImGui::BeginDragDropSource()) {
                        switch (i) {
                        case 0: case 1: ImGui::SetDragDropPayload("Player", &getConfigPlayer(i, items[j]), sizeof(Player), ImGuiCond_Once); break;
                        case 2: ImGui::SetDragDropPayload("Weapon", &config->weapons[items[j]], sizeof(Weapon), ImGuiCond_Once); break;
                        case 3: ImGui::SetDragDropPayload("Projectile", &config->projectiles[items[j]], sizeof(Projectile), ImGuiCond_Once); break;
                        default: ImGui::SetDragDropPayload("Entity", &getConfigShared(i, items[j]), sizeof(Shared), ImGuiCond_Once); break;
                        }
                        ImGui::EndDragDropSource();
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Player")) {
                            const auto& data = *(Player*)payload->Data;

                            switch (i) {
                            case 0: case 1: getConfigPlayer(i, items[j]) = data; break;
                            case 2: config->weapons[items[j]] = data; break;
                            case 3: config->projectiles[items[j]] = data; break;
                            default: getConfigShared(i, items[j]) = data; break;
                            }
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Weapon")) {
                            const auto& data = *(Weapon*)payload->Data;

                            switch (i) {
                            case 0: case 1: getConfigPlayer(i, items[j]) = data; break;
                            case 2: config->weapons[items[j]] = data; break;
                            case 3: config->projectiles[items[j]] = data; break;
                            default: getConfigShared(i, items[j]) = data; break;
                            }
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Projectile")) {
                            const auto& data = *(Projectile*)payload->Data;

                            switch (i) {
                            case 0: case 1: getConfigPlayer(i, items[j]) = data; break;
                            case 2: config->weapons[items[j]] = data; break;
                            case 3: config->projectiles[items[j]] = data; break;
                            default: getConfigShared(i, items[j]) = data; break;
                            }
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity")) {
                            const auto& data = *(Shared*)payload->Data;

                            switch (i) {
                            case 0: case 1: getConfigPlayer(i, items[j]) = data; break;
                            case 2: config->weapons[items[j]] = data; break;
                            case 3: config->projectiles[items[j]] = data; break;
                            default: getConfigShared(i, items[j]) = data; break;
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                if (i != 2)
                    continue;

                ImGui::Indent();

                const auto subItems = [](std::size_t item) noexcept -> std::vector<const char*> {
                    switch (item) {
                    case 0: return { "Glock-18", "P2000", "USP-S", "Dual Berettas", "P250", "Tec-9", "Five-SeveN", "CZ75-Auto", "Desert Eagle", "R8 Revolver" };
                    case 1: return { "MAC-10", "MP9", "MP7", "MP5-SD", "UMP-45", "P90", "PP-Bizon" };
                    case 2: return { "Galil AR", "FAMAS", "AK-47", "M4A4", "M4A1-S", "SG 553", "AUG" };
                    case 3: return { "SSG 08", "AWP", "G3SG1", "SCAR-20" };
                    case 4: return { "Nova", "XM1014", "Sawed-Off", "MAG-7" };
                    case 5: return { "M249", "Negev" };
                    case 6: return { "Flashbang", "HE Grenade", "Smoke Grenade", "Molotov", "Decoy Grenade", "Incendiary", "TA Grenade", "Fire Bomb", "Diversion", "Frag Grenade", "Snowball" };
                    case 7: return { "Axe", "Hammer", "Wrench" };
                    case 8: return { "C4", "Healthshot", "Bump Mine", "Zone Repulsor", "Shield" };
                    default: return { };
                    }
                }(j);

                const auto itemEnabled = getConfigShared(i, items[j]).enabled;

                for (const auto subItem : subItems) {
                    if ((categoryEnabled || itemEnabled) && !config->weapons[subItem].enabled)
                        continue;

                    if (ImGui::Selectable(subItem, currentCategory == i && selectedSubItem && std::string_view{ currentItem } == subItem)) {
                        currentCategory = i;
                        currentItem = subItem;
                        selectedSubItem = true;
                    }

                    if (ImGui::BeginDragDropSource()) {
                        ImGui::SetDragDropPayload("Weapon", &config->weapons[subItem], sizeof(Weapon), ImGuiCond_Once);
                        ImGui::EndDragDropSource();
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Player")) {
                            const auto& data = *(Player*)payload->Data;
                            config->weapons[subItem] = data;
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Weapon")) {
                            const auto& data = *(Weapon*)payload->Data;
                            config->weapons[subItem] = data;
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Projectile")) {
                            const auto& data = *(Projectile*)payload->Data;
                            config->weapons[subItem] = data;
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity")) {
                            const auto& data = *(Shared*)payload->Data;
                            config->weapons[subItem] = data;
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                ImGui::Unindent();
            }
            ImGui::Unindent();
            ImGui::PopID();
        }
        ImGui::ListBoxFooter();
    }

    ImGui::SameLine();

    if (ImGui::BeginChild("##child", { 400.0f, 0.0f })) {
        auto& sharedConfig = getConfigShared(currentCategory, currentItem);

        ImGui::Checkbox("Enabled", &sharedConfig.enabled);
        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 260.0f);
        ImGui::SetNextItemWidth(220.0f);
        if (ImGui::BeginCombo("Font", config->systemFonts[sharedConfig.font.index].c_str())) {
            for (size_t i = 0; i < config->systemFonts.size(); i++) {
                bool isSelected = config->systemFonts[i] == sharedConfig.font.name;
                if (ImGui::Selectable(config->systemFonts[i].c_str(), isSelected, 0, { 250.0f, 0.0f })) {
                    sharedConfig.font.index = i;
                    sharedConfig.font.name = config->systemFonts[i];
                    config->scheduleFontLoad(sharedConfig.font.name);
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();

        constexpr auto spacing = 220.0f;
        ImGuiCustom::colorPicker("Snapline", sharedConfig.snapline);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(90.0f);
        ImGui::Combo("##1", &sharedConfig.snapline.type, "Bottom\0Top\0Crosshair\0");
        ImGui::SameLine(spacing);
        ImGuiCustom::colorPicker("Box", sharedConfig.box);
        ImGui::SameLine();

        ImGui::PushID("Box");

        if (ImGui::Button("..."))
            ImGui::OpenPopup("");

        if (ImGui::BeginPopup("")) {
            ImGui::SetNextItemWidth(95.0f);
            ImGui::Combo("Type", &sharedConfig.box.type, "2D\0" "2D corners\0" "3D\0" "3D corners\0");
            ImGui::SetNextItemWidth(275.0f);
            ImGui::SliderFloat3("Scale", sharedConfig.box.scale.data(), 0.0f, 0.50f, "%.2f");
            ImGui::EndPopup();
        }

        ImGui::PopID();

        ImGuiCustom::colorPicker("Name", sharedConfig.name);
        ImGui::SameLine(spacing);
        ImGuiCustom::colorPicker("Text Background", sharedConfig.textBackground);
        ImGui::SetNextItemWidth(95.0f);
        ImGui::InputFloat("Text Cull Distance", &sharedConfig.textCullDistance, 0.4f, 0.8f, "%.1fm");
        sharedConfig.textCullDistance = std::clamp(sharedConfig.textCullDistance, 0.0f, 999.9f);

        if (currentCategory < 2) {
            auto& playerConfig = getConfigPlayer(currentCategory, currentItem);

            ImGuiCustom::colorPicker("Weapon", playerConfig.weapon);
            ImGui::SameLine(spacing);
            ImGuiCustom::colorPicker("Flash Duration", playerConfig.flashDuration);
            ImGui::Checkbox("Audible Only", &playerConfig.audibleOnly);
            ImGui::SameLine(spacing);
            ImGuiCustom::colorPicker("Skeleton", playerConfig.skeleton);
            ImGui::Checkbox("Spotted Only", &playerConfig.spottedOnly);
        } else if (currentCategory == 2) {
            auto& weaponConfig = config->weapons[currentItem];

           // if (currentItem != 7)
                ImGuiCustom::colorPicker("Ammo", weaponConfig.ammo);
        } else if (currentCategory == 3) {
            ImGui::Checkbox("Trails", &config->projectiles[currentItem].trails.enabled);
            ImGui::SameLine();
           
            if (ImGui::Button("..."))
                ImGui::OpenPopup("##trails");

            if (ImGui::BeginPopup("##trails")) {
                constexpr auto trailPicker = [](const char* name, Trail& trail) noexcept {
                    ImGui::PushID(name);
                    ImGuiCustom::colorPicker(name, trail);
                    ImGui::SameLine(150.0f);
                    ImGui::SetNextItemWidth(95.0f);
                    ImGui::Combo("", &trail.type, "Line\0Circles\0Filled Circles\0");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(95.0f);
                    ImGui::InputFloat("Time", &trail.time, 0.1f, 0.5f, "%.1fs");
                    trail.time = std::clamp(trail.time, 1.0f, 60.0f);
                    ImGui::PopID();
                };

                trailPicker("Local Player", config->projectiles[currentItem].trails.localPlayer);
                trailPicker("Allies", config->projectiles[currentItem].trails.allies);
                trailPicker("Enemies", config->projectiles[currentItem].trails.enemies);
                ImGui::EndPopup();
            }
        }
    }

    ImGui::EndChild();
}

// future implementation

bool drawPlayerCategories(const char* (&currentCategory), const char* (&currentItem)) noexcept
{
    bool selected = false;

    constexpr auto dragDrop = [](Player& player) {
        if (ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("Player", &player, sizeof(Player), ImGuiCond_Once);
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Player"))
                player = *(Player*)payload->Data;

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Weapon"))
                player = *(Weapon*)payload->Data;

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Projectile"))
                player = *(Projectile*)payload->Data;

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
                player = *(Shared*)payload->Data;

            ImGui::EndDragDropTarget();
        }
    };

    auto category = [&](const char* name, std::unordered_map<std::string, Player>& cfg) {
        if (ImGui::Selectable(name, std::string_view{ currentCategory } == name && std::string_view{ currentItem } == "All")) {
            currentCategory = name;
            currentItem = "All";
            selected = true;
        }

        dragDrop(cfg["All"]);

        ImGui::Indent();

        for (const auto item : std::array{ "Visible", "Occluded" }) {
            if (!cfg["All"].enabled || cfg[item].enabled) {
                if (ImGui::Selectable(item, std::string_view{ currentCategory } == name && std::string_view{ currentItem } == item)) {
                    currentCategory = name;
                    currentItem = item;
                    selected = true;
                }
                dragDrop(cfg[item]);
            }
        }

        ImGui::Unindent();
    };

    category("Allies", config->allies);
    category("Enemies", config->enemies);

    return selected;
}

bool drawWeaponCategories(const char* (&currentCategory), const char* (&currentItem)) noexcept
{
    bool selected = false;

    constexpr auto dragDrop = [](Weapon& weapon) {
        if (ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("Weapon", &weapon, sizeof(Weapon), ImGuiCond_Once);
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Player"))
                weapon = *(Player*)payload->Data;

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Weapon"))
                weapon = *(Weapon*)payload->Data;

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Projectile"))
                weapon = *(Projectile*)payload->Data;

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
                weapon = *(Shared*)payload->Data;

            ImGui::EndDragDropTarget();
        }
    };

    if (ImGui::Selectable("Weapons", std::string_view{ currentCategory } == "Weapons" && std::string_view{ currentItem } == "All")) {
        currentCategory = "Weapons";
        currentItem = "All";
        selected = true;
    }

    dragDrop(config->weapons["All"]);

    ImGui::PushID("Weapons");
    ImGui::Indent();

    constexpr std::array items{ "Pistols", "SMGs", "Rifles", "Sniper Rifles", "Shotguns", "Machineguns", "Grenades", "Melee", "Other" };

    for (std::size_t i = 0; i < items.size(); ++i) {
        if (!config->weapons["All"].enabled || config->weapons[items[i]].enabled) {
            if (ImGui::Selectable(items[i], std::string_view{ currentCategory } == "Weapons" && std::string_view{ currentItem } == items[i])) {
                currentCategory = "Weapons";
                currentItem = items[i];
                selected = true;
            }

            dragDrop(config->weapons[items[i]]);
        }

        const auto subItems = [](std::size_t item) noexcept -> std::vector<const char*> {
            switch (item) {
            case 0: return { "Glock-18", "P2000", "USP-S", "Dual Berettas", "P250", "Tec-9", "Five-SeveN", "CZ75-Auto", "Desert Eagle", "R8 Revolver" };
            case 1: return { "MAC-10", "MP9", "MP7", "MP5-SD", "UMP-45", "P90", "PP-Bizon" };
            case 2: return { "Galil AR", "FAMAS", "AK-47", "M4A4", "M4A1-S", "SG 553", "AUG" };
            case 3: return { "SSG 08", "AWP", "G3SG1", "SCAR-20" };
            case 4: return { "Nova", "XM1014", "Sawed-Off", "MAG-7" };
            case 5: return { "M249", "Negev" };
            case 6: return { "Flashbang", "HE Grenade", "Smoke Grenade", "Molotov", "Decoy Grenade", "Incendiary", "TA Grenade", "Fire Bomb", "Diversion", "Frag Grenade", "Snowball" };
            case 7: return { "Axe", "Hammer", "Wrench" };
            case 8: return { "C4", "Healthshot", "Bump Mine", "Zone Repulsor", "Shield" };
            default: return { };
            }
        }(i);

        ImGui::Indent();
        for (const auto subItem : subItems) {
            if ((config->weapons["All"].enabled || config->weapons[items[i]].enabled) && !config->weapons[subItem].enabled)
                continue;

            if (ImGui::Selectable(subItem, std::string_view{ currentCategory } == "Weapons" && std::string_view{ currentItem } == subItem)) {
                currentCategory = "Weapons";
                currentItem = subItem;
                selected = true;
            }

            dragDrop(config->weapons[subItem]);
        }
        ImGui::Unindent();
    }

    ImGui::Unindent();
    ImGui::PopID();
    return selected;
}
