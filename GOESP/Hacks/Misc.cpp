#include "Misc.h"

#include "../imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui_internal.h"

#include "../Config.h"
#include "../fnv.h"
#include "../GameData.h"
#include "../GUI.h"
#include "../Helpers.h"
#include "../Interfaces.h"
#include "../Memory.h"
#include "../SDK/ClientClass.h"
#include "../SDK/ConVar.h"
#include "../SDK/Cvar.h"
#include "../SDK/Engine.h"
#include "../SDK/Entity.h"
#include "../SDK/EntityList.h"
#include "../SDK/GameEvent.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/ItemSchema.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/NetworkChannel.h"
#include "../SDK/WeaponInfo.h"
#include "../SDK/WeaponSystem.h"

#include <numbers>
#include <numeric>
#include <unordered_map>
#include <vector>
#include <sstream>

void Misc::drawReloadProgress(ImDrawList* drawList) noexcept
{
    if (!config->reloadProgress.enabled)
        return;

    GameData::Lock lock;
    const auto& localPlayerData = GameData::local();

    if (!localPlayerData.exists || !localPlayerData.alive)
        return;

    static float reloadLength = 0.0f;

    if (localPlayerData.inReload) {
        if (!reloadLength)
            reloadLength = localPlayerData.nextWeaponAttack - memory->globalVars->currenttime;

        constexpr int segments = 40;
        constexpr float pi = std::numbers::pi_v<float>;
        constexpr float min = -pi / 2;
        const float max = std::clamp(pi * 2 * (0.75f - (localPlayerData.nextWeaponAttack - memory->globalVars->currenttime) / reloadLength), -pi / 2, -pi / 2 + pi * 2);

        drawList->PathArcTo(ImGui::GetIO().DisplaySize / 2.0f + ImVec2{ 1.0f, 1.0f }, 20.0f, min, max, segments);
        const ImU32 color = Helpers::calculateColor(config->reloadProgress);
        drawList->PathStroke(color & 0xFF000000, false, config->reloadProgress.thickness);
        drawList->PathArcTo(ImGui::GetIO().DisplaySize / 2.0f, 20.0f, min, max, segments);
        drawList->PathStroke(color, false, config->reloadProgress.thickness);
    } else {
        reloadLength = 0.0f;
    }
}

static void drawCrosshair(ImDrawList* drawList, const ImVec2& pos, ImU32 color, float thickness) noexcept
{
    // dot
    drawList->AddRectFilled(pos - ImVec2{ 1, 1 }, pos + ImVec2{ 2, 2 }, color & IM_COL32_A_MASK);
    drawList->AddRectFilled(pos, pos + ImVec2{ 1, 1 }, color);

    // left
    drawList->AddRectFilled(ImVec2{ pos.x - 11, pos.y - 1 }, ImVec2{ pos.x - 3, pos.y + 2 }, color & IM_COL32_A_MASK);
    drawList->AddRectFilled(ImVec2{ pos.x - 10, pos.y }, ImVec2{ pos.x - 4, pos.y + 1 }, color);

    // right
    drawList->AddRectFilled(ImVec2{ pos.x + 4, pos.y - 1 }, ImVec2{ pos.x + 12, pos.y + 2 }, color & IM_COL32_A_MASK);
    drawList->AddRectFilled(ImVec2{ pos.x + 5, pos.y }, ImVec2{ pos.x + 11, pos.y + 1 }, color);

    // top (left with swapped x/y offsets)
    drawList->AddRectFilled(ImVec2{ pos.x - 1, pos.y - 11 }, ImVec2{ pos.x + 2, pos.y - 3 }, color & IM_COL32_A_MASK);
    drawList->AddRectFilled(ImVec2{ pos.x, pos.y - 10 }, ImVec2{ pos.x + 1, pos.y - 4 }, color);

    // bottom (right with swapped x/y offsets)
    drawList->AddRectFilled(ImVec2{ pos.x - 1, pos.y + 4 }, ImVec2{ pos.x + 2, pos.y + 12 }, color & IM_COL32_A_MASK);
    drawList->AddRectFilled(ImVec2{ pos.x, pos.y + 5 }, ImVec2{ pos.x + 1, pos.y + 11 }, color);
}

void Misc::drawRecoilCrosshair(ImDrawList* drawList) noexcept
{
    if (!config->recoilCrosshair.enabled)
        return;

    GameData::Lock lock;
    const auto& localPlayerData = GameData::local();

    if (!localPlayerData.exists || !localPlayerData.alive)
        return;

    if (!localPlayerData.shooting)
        return;

    auto pos = ImGui::GetIO().DisplaySize;
    pos.x *= 0.5f - localPlayerData.aimPunch.y / (localPlayerData.fov * 2.0f);
    pos.y *= 0.5f + localPlayerData.aimPunch.x / (localPlayerData.fov * 2.0f);

    drawCrosshair(drawList, pos, Helpers::calculateColor(config->recoilCrosshair), config->recoilCrosshair.thickness);
}

void Misc::purchaseList(GameEvent* event) noexcept
{
    static std::mutex mtx;
    std::scoped_lock _{ mtx };

    struct PlayerPurchases {
        int totalCost;
        std::unordered_map<std::string, int> items;
    };

    static std::unordered_map<int, PlayerPurchases> playerPurchases;
    static std::unordered_map<std::string, int> purchaseTotal;
    static int totalCost;

    static auto freezeEnd = 0.0f;

    if (event) {
        switch (fnv::hashRuntime(event->getName())) {
        case fnv::hash("item_purchase"): {
            const auto player = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserId(event->getInt("userid")));
            if (!player || !localPlayer || !memory->isOtherEnemy(player, localPlayer.get()))
                break;

            const auto weaponName = event->getString("weapon");

#ifdef __APPLE__
            if (const auto definition = (*memory->itemSystem)->getItemSchema()->getItemDefinitionByName(weaponName)) {
#else
            if (const auto definition = memory->itemSystem()->getItemSchema()->getItemDefinitionByName(weaponName)) {
#endif
                if (const auto weaponInfo = memory->weaponSystem->getWeaponInfo(definition->getWeaponId())) {
                    auto& purchase = playerPurchases[player->getUserId()];

                    purchase.totalCost += weaponInfo->price;
                    totalCost += weaponInfo->price;

                    std::string weapon = weaponName;

                    if (weapon.starts_with("weapon_"))
                        weapon.erase(0, 7);
                    else if (weapon.starts_with("item_"))
                        weapon.erase(0, 5);

                    if (weapon.starts_with("smoke"))
                        weapon.erase(5);
                    else if (weapon.starts_with("m4a1_s"))
                        weapon.erase(6);
                    else if (weapon.starts_with("usp_s"))
                        weapon.erase(5);

                    ++purchase.items[weapon];
                    ++purchaseTotal[weapon];
                }
            }

            break;
        }
        case fnv::hash("round_start"):
            freezeEnd = 0.0f;
            playerPurchases.clear();
            purchaseTotal.clear();
            totalCost = 0;
            break;
        case fnv::hash("round_freeze_end"):
            freezeEnd = memory->globalVars->realtime;
            break;
        }
    } else {
        if (!config->purchaseList.enabled)
            return;

        static const auto mp_buytime = interfaces->cvar->findVar("mp_buytime");

        if ((!interfaces->engine->isInGame() || (freezeEnd != 0.0f && memory->globalVars->realtime > freezeEnd + (!config->purchaseList.onlyDuringFreezeTime ? mp_buytime->getFloat() : 0.0f)) || playerPurchases.empty() || purchaseTotal.empty()) && !gui->open)
            return;

        if (config->purchaseList.pos != ImVec2{}) {
            ImGui::SetNextWindowPos(config->purchaseList.pos);
            config->purchaseList.pos = {};
        }

        if (config->purchaseList.size != ImVec2{}) {
            ImGui::SetNextWindowSize(ImClamp(config->purchaseList.size, {}, ImGui::GetIO().DisplaySize));
            config->purchaseList.size = {};
        }

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
        if (!gui->open)
            windowFlags |= ImGuiWindowFlags_NoInputs;
        if (config->purchaseList.noTitleBar)
            windowFlags |= ImGuiWindowFlags_NoTitleBar;

        ImGui::Begin("Purchases", nullptr, windowFlags);

        if (config->purchaseList.mode == PurchaseList::Details) {
            GameData::Lock lock;

            for (const auto& [userId, purchases] : playerPurchases) {
                std::string s;
                s.reserve(std::accumulate(purchases.items.begin(), purchases.items.end(), 0, [](int length, const auto& p) { return length + p.first.length() + 2; }));
                for (const auto& purchasedItem : purchases.items) {
                    if (purchasedItem.second > 1)
                        s += std::to_string(purchasedItem.second) + "x ";
                    s += purchasedItem.first + ", ";
                }

                if (s.length() >= 2)
                    s.erase(s.length() - 2);

                if (const auto it = std::find_if(GameData::players().cbegin(), GameData::players().cend(), [userId = userId](const auto& playerData) { return playerData.userId == userId; }); it != GameData::players().cend()) {
                    if (config->purchaseList.showPrices)
                        ImGui::TextWrapped("%s $%d: %s", it->name, purchases.totalCost, s.c_str());
                    else
                        ImGui::TextWrapped("%s: %s", it->name, s.c_str());
                }
            }
        } else if (config->purchaseList.mode == PurchaseList::Summary) {
            for (const auto& purchase : purchaseTotal)
                ImGui::TextWrapped("%dx %s", purchase.second, purchase.first.c_str());

            if (config->purchaseList.showPrices && totalCost > 0) {
                ImGui::Separator();
                ImGui::TextWrapped("Total: $%d", totalCost);
            }
        }
        ImGui::End();
    }
}

void Misc::drawObserverList() noexcept
{
    if (!config->observerList.enabled)
        return;

    GameData::Lock lock;

    const auto& observers = GameData::observers();

    if (std::none_of(observers.begin(), observers.end(), [](const auto& obs) { return obs.targetIsLocalPlayer; }) && !gui->open)
        return;

    if (config->observerList.pos != ImVec2{}) {
        ImGui::SetNextWindowPos(config->observerList.pos);
        config->observerList.pos = {};
    }

    if (config->observerList.size != ImVec2{}) {
        ImGui::SetNextWindowSize(ImClamp(config->observerList.size, {}, ImGui::GetIO().DisplaySize));
        config->observerList.size = {};
    }

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
    if (!gui->open)
        windowFlags |= ImGuiWindowFlags_NoInputs;
    if (config->observerList.noTitleBar)
        windowFlags |= ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Observer List", nullptr, windowFlags);

    for (const auto& observer : observers) {
        if (!observer.targetIsLocalPlayer)
            continue;

        ImGui::TextWrapped("%s", observer.name);
    }

    ImGui::End();
}

void Misc::drawNoscopeCrosshair(ImDrawList* drawList) noexcept
{
    if (!config->noscopeCrosshair.enabled)
        return;

    GameData::Lock lock;
    const auto& localPlayerData = GameData::local();

    if (!localPlayerData.exists || !localPlayerData.alive)
        return;

    if (!localPlayerData.noScope)
        return;

    drawCrosshair(drawList, ImGui::GetIO().DisplaySize / 2, Helpers::calculateColor(config->noscopeCrosshair), config->noscopeCrosshair.thickness);
}

void Misc::drawFpsCounter() noexcept
{
    if (!config->fpsCounter.enabled)
        return;

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
    if (!gui->open)
        windowFlags |= ImGuiWindowFlags_NoInputs;

    ImGui::SetNextWindowBgAlpha(0.35f);
    ImGui::Begin("FPS Counter", nullptr, windowFlags);

    static auto frameRate = 1.0f;
    frameRate = 0.9f * frameRate + 0.1f * memory->globalVars->absoluteFrameTime;
    if (frameRate != 0.0f)
        ImGui::Text("%d fps", static_cast<int>(1 / frameRate));

    ImGui::End();
}

void Misc::drawOffscreenEnemies(ImDrawList* drawList) noexcept
{
    if (!config->offscreenEnemies.enabled)
        return;

    GameData::Lock lock;

    for (auto& player : GameData::players()) {
        if (player.dormant || !player.alive || !player.enemy || player.inViewFrustum)
            continue;

        const auto positionDiff = GameData::local().origin - player.origin;
        const auto yaw = Helpers::deg2rad(interfaces->engine->getViewAngles().y);

        auto x = std::cos(yaw) * positionDiff.y - std::sin(yaw) * positionDiff.x;
        auto y = std::cos(yaw) * positionDiff.x + std::sin(yaw) * positionDiff.y;
        const auto len = std::sqrt(x * x + y * y);
        x /= len;
        y /= len;

        const auto pos = ImGui::GetIO().DisplaySize / 2 + ImVec2{ x, y } * 200;
        const auto color = Helpers::calculateColor(config->offscreenEnemies.color);
        drawList->AddCircleFilled(pos, 11.0f, color & IM_COL32_A_MASK, 40);
        drawList->AddCircleFilled(pos, 10.0f, color, 40);
    }
}

auto ConvertRGB(float mult, float R, float G, float B, float A, float scale)
{
    float H, S, V;
    ImGui::ColorConvertRGBtoHSV(R, G, B, H, S, V);
    if ((H + (mult * scale)) > 1.0f)
        H = (mult * scale) - (1.0f - H);
    else
        H += mult * scale;
    ImGui::ColorConvertHSVtoRGB(H, S, V, R, G, B);
    return ImGui::ColorConvertFloat4ToU32({ R, G, B, A });
}

void Misc::rainbowBar(ImDrawList* drawList)noexcept
{
    if (!config->rainbowBar.enabled)
        return;

    float colorR = 0;
    float colorG = 0;
    float colorB = 0;
    if (config->rainbowBar.rainbow) {
        colorR = std::sin(config->rainbowBar.rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f;
        colorG = std::sin(config->rainbowBar.rainbowSpeed * memory->globalVars->realtime + 2 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f;
        colorB = std::sin(config->rainbowBar.rainbowSpeed * memory->globalVars->realtime + 4 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f;
    }
    else {
        colorR = config->rainbowBar.color[0];
        colorG = config->rainbowBar.color[1];
        colorB = config->rainbowBar.color[2];
    }
    float colorA = config->rainbowBar.color[3];
    float tickness = config->rainbowBar.thickness;
    float scale = config->rainbowScale;
    float pulse, pulseAlpha;
    if (config->rainbowPulse) {
        pulse = std::sin(config->rainbowPulseSpeed * memory->globalVars->realtime) * 0.5f + 0.5f;
        pulseAlpha = (std::sin(config->rainbowPulseSpeed * memory->globalVars->realtime) * 0.5f + 0.5f) * colorA;
    }
    else {
        pulse = 1.0f;
        pulseAlpha = colorA;
    }

    ImVec2 zero = { 0,0 };
    ImVec2 ds = ImGui::GetIO().DisplaySize;

    ImU32 red = ConvertRGB(0, colorR, colorG, colorB, pulse, scale);
    ImU32 amber = ConvertRGB(1, colorR, colorG, colorB, pulse, scale);
    ImU32 chartreuse = ConvertRGB(2, colorR, colorG, colorB, pulse, scale);
    ImU32 malachite = ConvertRGB(3, colorR, colorG, colorB, pulse, scale);
    ImU32 cyan = ConvertRGB(4, colorR, colorG, colorB, pulse, scale);
    ImU32 blue = ConvertRGB(5, colorR, colorG, colorB, pulse, scale);
    ImU32 indigo = ConvertRGB(6, colorR, colorG, colorB, pulse, scale);
    ImU32 magenta = ConvertRGB(7, colorR, colorG, colorB, pulse, scale);
    ImU32 red0 = ConvertRGB(0, colorR, colorG, colorB, pulseAlpha, scale);
    ImU32 amber0 = ConvertRGB(1, colorR, colorG, colorB, pulseAlpha, scale);
    ImU32 chartreuse0 = ConvertRGB(2, colorR, colorG, colorB, pulseAlpha, scale);
    ImU32 malachite0 = ConvertRGB(3, colorR, colorG, colorB, pulseAlpha, scale);
    ImU32 cyan0 = ConvertRGB(4, colorR, colorG, colorB, pulseAlpha, scale);
    ImU32 blue0 = ConvertRGB(5, colorR, colorG, colorB, pulseAlpha, scale);
    ImU32 indigo0 = ConvertRGB(6, colorR, colorG, colorB, pulseAlpha, scale);
    ImU32 magenta0 = ConvertRGB(7, colorR, colorG, colorB, pulseAlpha, scale);

    if (tickness > ds.y) {
        config->rainbowBar.thickness = ds.y;
        tickness = ds.y;
    }

    //drawList->AddRectFilledMultiColor(upper - left, lower - right, Color Upper Left, Color Upper Right, Color Bottom Right, Color Bottom Left);

    if (config->rainbowBottom) {
        // Bottom
        drawList->AddRectFilledMultiColor({ zero.x, ds.y - tickness }, { ds.x / 2, ds.y }, indigo0, blue0, blue, indigo);
        drawList->AddRectFilledMultiColor({ ds.x / 2, ds.y - tickness }, { ds.x, ds.y }, blue0, cyan0, cyan, blue);
    }
    if (config->rainbowLeft) {
        // Left
        drawList->AddRectFilledMultiColor(zero, { tickness, ds.y / 2 }, red, red0, magenta0, magenta);
        drawList->AddRectFilledMultiColor({ zero.x, ds.y / 2 }, { tickness, ds.y }, magenta, magenta0, indigo0, indigo);
    }
    if (config->rainbowRight) {
        // Right
        drawList->AddRectFilledMultiColor({ ds.x - tickness, zero.y }, { ds.x, ds.y / 2 }, chartreuse0, chartreuse, malachite, malachite0);
        drawList->AddRectFilledMultiColor({ ds.x - tickness, ds.y / 2 }, ds, malachite0, malachite, cyan, cyan0);
    }
    if (config->rainbowUp) {
        // Upper
        drawList->AddRectFilledMultiColor(zero, { ds.x / 2, tickness + (0.0f) }, red, amber, amber0, red0);
        drawList->AddRectFilledMultiColor({ ds.x / 2, zero.y }, { ds.x, tickness + (0.0f) }, amber, chartreuse, chartreuse0, amber0);
    }
}

void Misc::watermark() noexcept
{
    if (config->watermark.enabled) {
        std::string watermark = "GOESP BETA";

        if (interfaces->engine->isInGame() && config->watermarkNickname) {
            PlayerInfo playerInfo;
            auto nickname = interfaces->engine->getPlayerInfo(localPlayer->index(), playerInfo);
            watermark.append(" | ").append(playerInfo.name);
        };

        if (config->watermarkUsername)
            watermark.append(" | ")
#ifdef _WIN32
            .append(getenv("USERNAME"));
#else
            .append(getenv("USER"));
#endif

        if (config->watermarkFPS) {
            static auto frameRate = 1.0f;
            frameRate = 0.9f * frameRate + 0.1f * memory->globalVars->absoluteFrameTime;
            watermark.append(" | FPS: ").append(std::to_string(static_cast<int>(1 / frameRate)));
        }

        if (config->watermarkPing) {
            float latency = 0.0f;
            if (auto networkChannel = interfaces->engine->getNetworkChannel(); networkChannel && networkChannel->getLatency(0) > 0.0f)
                latency = networkChannel->getLatency(0);
            watermark.append(" | Ping: ").append(std::to_string(static_cast<int>(latency * 1000))).append(" ms");
        }

        if (config->watermarkTickrate)
            watermark.append(" | ").append(std::to_string(static_cast<int>(1.0f / memory->globalVars->intervalPerTick))).append(" tick");

        if (config->watermarkVelocity && localPlayer && localPlayer->isAlive())
            watermark.append(" | ").append(std::to_string(static_cast<int>(round(localPlayer->velocity().length2D())))).append(" ups");

        if (config->watermarkTime) {
            const auto time = std::time(nullptr);
            const auto localTime = std::localtime(&time);
            std::ostringstream timeShow;
            timeShow << std::setfill('0') << std::setw(2) << localTime->tm_hour << ":" << std::setw(2) << localTime->tm_min << ":" << std::setw(2) << localTime->tm_sec;
            watermark.append(" | ").append(timeShow.str());
        }

        auto pos = config->watermarkPos * ImGui::GetIO().DisplaySize;

        ImGuiCond nextFlag = ImGuiCond_None;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f }, ImGuiCond_Always);
        if (ImGui::IsMouseDown(0))
            nextFlag |= ImGuiCond_Once;
        else
            nextFlag |= ImGuiCond_Always;
        ImGui::SetNextWindowPos(pos, nextFlag);

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        if (!gui->open)
            windowFlags |= ImGuiWindowFlags_NoInputs;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, { 0.5f, 0.5f });
        ImGui::Begin("Watermark", nullptr, windowFlags);
        ImGui::PopStyleVar();

        auto [x, y] = ImGui::GetWindowPos();
        auto [w, h] = ImGui::GetWindowSize();
        auto ds = ImGui::GetIO().DisplaySize;

        /// Avoid to move window out of screen by right and bottom border
        if (x > (ds.x - w) && y > (ds.y - h)) {
            x = ds.x - w;
            y = ds.y - h;
        }
        else if (x > (ds.x - w) && y <= (ds.y - h))
            x = ds.x - w;
        else if (x <= (ds.x - w) && y > (ds.y - h))
            y = ds.y - h;

        /// Avoid to move window out of screen by left and top border
        if (x < 0 && y < 0) {
            x = 0;
            y = 0;
        }
        else if (x < 0 && y >= 0)
            x = 0;
        else if (x >= 0 && y < 0)
            y = 0;

        /// Save pos in float 0.f - 0, 1.f - Display size
        /// in 1920x1080 float 0.5f X and 0.125f Y will be 960x135
        config->watermarkPos = ImVec2{ x / ds.x ,y / ds.y };

        ImGui::SetWindowFontScale(config->watermarkScale);
        if (config->watermark.rainbow) {
            auto colorR = std::sin(config->watermark.rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f;
            auto colorG = std::sin(config->watermark.rainbowSpeed * memory->globalVars->realtime + 2 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f;
            auto colorB = std::sin(config->watermark.rainbowSpeed * memory->globalVars->realtime + 4 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f;
#ifndef _WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#endif
            ImGui::TextColored({ colorR, colorG, colorB, 1.0f }, watermark.c_str());
#ifndef _WIN32
#pragma GCC diagnostic pop
#endif
        }
        else
#ifndef _WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#endif
            ImGui::TextColored({ config->watermark.color[0], config->watermark.color[1] ,config->watermark.color[2], 1.0f }, watermark.c_str());
#ifndef _WIN32
#pragma GCC diagnostic pop
#endif

        ImGui::End();
    }
}

void Misc::hitMarker(GameEvent* event) noexcept
{
    if (!config->hitMarker.enabled || !localPlayer)
        return;

    static float lastHitTime = 0.0f;

    if (event && interfaces->engine->getPlayerForUserId(event->getInt("attacker")) == localPlayer->index()) {
        lastHitTime = memory->globalVars->realtime;
        return;
    }

    if (lastHitTime + config->hitMarkerTime < memory->globalVars->realtime)
        return;

    const auto ds = ImGui::GetIO().DisplaySize;

    auto start = 4;
    const auto width_mid = ds.x / 2;
    const auto height_mid = ds.y / 2;

    auto drawList = ImGui::GetBackgroundDrawList();
    ImU32 color = Helpers::calculateColor(config->hitMarker);
    drawList->AddLine({ width_mid + config->hitMarkerLength, height_mid + config->hitMarkerLength }, { width_mid + start, height_mid + start }, color, config->hitMarker.thickness);
    drawList->AddLine({ width_mid - config->hitMarkerLength, height_mid + config->hitMarkerLength }, { width_mid - start, height_mid + start }, color, config->hitMarker.thickness);
    drawList->AddLine({ width_mid + config->hitMarkerLength, height_mid - config->hitMarkerLength }, { width_mid + start, height_mid - start }, color, config->hitMarker.thickness);
    drawList->AddLine({ width_mid - config->hitMarkerLength, height_mid - config->hitMarkerLength }, { width_mid - start, height_mid - start }, color, config->hitMarker.thickness);
}

struct HitMarkerInfo {
    float hitMarkerExpTime;
    int hitMarkerDmg;
};

std::vector<HitMarkerInfo> hitMarkerInfo;

void Misc::hitMarkerSetDamageIndicator(GameEvent* event) noexcept {
    if (!localPlayer)
        return;

    if (config->hitMarkerDamageIndicator.enabled)
        if (event && interfaces->engine->getPlayerForUserId(event->getInt("attacker")) == localPlayer->index())
            hitMarkerInfo.push_back({ memory->globalVars->realtime + config->hitMarkerTime, event->getInt("dmg_health") });
}

void Misc::hitMarkerDamageIndicator() noexcept
{
    if (config->hitMarkerDamageIndicator.enabled) {
        if (hitMarkerInfo.empty()) return;

        const auto ds = ImGui::GetIO().DisplaySize;

        for (size_t i = 0; i < hitMarkerInfo.size(); i++) {
            const auto diff = hitMarkerInfo.at(i).hitMarkerExpTime - memory->globalVars->realtime;

            if (diff < 0.f) {
                hitMarkerInfo.erase(hitMarkerInfo.begin() + i);
                continue;
            }

            const auto dist = config->hitMarkerDamageIndicatorCustomize ? config->hitMarkerDamageIndicatorDist : 50;
            const auto ratio = (config->hitMarkerDamageIndicatorCustomize ? config->hitMarkerDamageIndicatorRatio : 0.6f) - diff;

            auto drawList = ImGui::GetBackgroundDrawList();
            ImU32 color = Helpers::calculateColor(config->hitMarkerDamageIndicator);
            drawList->AddText({ ds.x / 2 + (config->hitMarker.enabled ? config->hitMarkerLength + 2 : 2) + ratio * dist / 2, ds.y / 2 + (config->hitMarker.enabled ? config->hitMarkerLength + 2 : 2) + ratio * dist }, color, std::to_string(hitMarkerInfo.at(i).hitMarkerDmg).c_str());
        }
    }
}

void Misc::drawBombTimer() noexcept
{
    if (config->bombTimer.enabled) {
        for (int i = interfaces->engine->getMaxClients(); i <= interfaces->entityList->getHighestEntityIndex(); i++) {
            Entity* entity = interfaces->entityList->getEntity(i);
            if (!entity || entity->isDormant() || entity->getClientClass()->classId != ClassId::PlantedC4 || !entity->bombTicking())
                continue;

            auto drawList = ImGui::GetBackgroundDrawList();

            std::stringstream bombText;
            bombText << "Bomb on " << (!entity->bombSite() ? 'A' : 'B') << " : " <<
                std::fixed << std::showpoint << std::setprecision(3) <<
                (std::max)(entity->c4Blow() - memory->globalVars->currenttime, 0.0f) << " s";

            auto drawPositionY{ ImGui::GetIO().DisplaySize.y / 8 + ImGui::CalcTextSize(bombText.str().c_str()).y };
            const auto bombTextX{ ImGui::GetIO().DisplaySize.x / 2 - (ImGui::CalcTextSize(bombText.str().c_str())).x / 2 };

            drawList->AddText({ ImGui::GetIO().DisplaySize.x / 2 - (ImGui::CalcTextSize(bombText.str().c_str())).x / 2, drawPositionY },
                IM_COL32(255, 255, 255, 255),
                bombText.str().c_str());

            const auto progressBarX{ ImGui::GetIO().DisplaySize.x / 3 };
            const auto progressBarLength{ ImGui::GetIO().DisplaySize.x / 3 };
            constexpr auto progressBarHeight{ 5 };

            drawList->AddRectFilled({ progressBarX - 3, drawPositionY + 2 },
                { progressBarX + progressBarLength + 3, drawPositionY + progressBarHeight + 8 },
                IM_COL32(50, 50, 50, 255));

            static auto c4Timer = interfaces->cvar->findVar("mp_c4timer");
            drawList->AddRectFilled({ progressBarX, drawPositionY + 5 },
                { progressBarX + progressBarLength * std::clamp(entity->c4Blow() - memory->globalVars->currenttime,
                    0.0f, c4Timer->getFloat()) / c4Timer->getFloat(), drawPositionY + progressBarHeight + 5 },
                Helpers::calculateColor(config->bombTimer));

            if (entity->bombDefuser() != -1) {
                if (PlayerInfo playerInfo; interfaces->engine->getPlayerInfo(interfaces->entityList->getEntityFromHandle(entity->bombDefuser())->index(), playerInfo)) {
                    drawPositionY += ImGui::CalcTextSize(" ").y;

                    std::stringstream defusingText;
                    defusingText << playerInfo.name << " is defusing: " <<
                        std::fixed << std::showpoint << std::setprecision(3) <<
                        (std::max)(entity->defuseCountDown() - memory->globalVars->currenttime, 0.0f) << " s";

                    drawList->AddText({ (ImGui::GetIO().DisplaySize.x - ImGui::CalcTextSize(defusingText.str().c_str()).x) / 2, drawPositionY },
                        Helpers::calculateColor(config->bombTimer),
                        defusingText.str().c_str());

                    drawPositionY += ImGui::CalcTextSize(" ").y;

                    drawList->AddRectFilled({ progressBarX - 3, drawPositionY + 2 },
                        { progressBarX + progressBarLength + 3, drawPositionY + progressBarHeight + 8 },
                        IM_COL32(50, 50, 50, 255));

                    drawList->AddRectFilled({ progressBarX, drawPositionY + 5 },
                        { progressBarX + progressBarLength *
                            (std::max)(entity->defuseCountDown() - memory->globalVars->currenttime, 0.0f) /
                            (interfaces->entityList->getEntityFromHandle(entity->bombDefuser())->hasDefuser() ? 5.0f : 10.0f),
                        drawPositionY + progressBarHeight + 5 },
                        IM_COL32(0, 255, 0, 255));

                    drawPositionY += ImGui::CalcTextSize(" ").y;
                    const char* canDefuseText;
                    ImU32 defcol = 0;

                    if (entity->c4Blow() >= entity->defuseCountDown()) {
                        canDefuseText = "Can Defuse";
                        defcol = IM_COL32(0, 255, 0, 255);
                    }
                    else {
                        canDefuseText = "Cannot Defuse";
                        defcol = IM_COL32(255, 0, 0, 255);
                    }
                    drawList->AddText({ (ImGui::GetIO().DisplaySize.x - ImGui::CalcTextSize(canDefuseText).x) / 2, drawPositionY }, defcol, canDefuseText);
                }
            }
            break;
        }
    }
}
