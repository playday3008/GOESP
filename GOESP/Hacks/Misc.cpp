#include "Misc.h"

#include "../imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui_internal.h"

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
#include "../SDK/Localize.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/WeaponInfo.h"
#include "../SDK/WeaponSystem.h"

#include "../ImGuiCustom.h"
#include "../imgui/imgui.h"

#include <numbers>
#include <numeric>
#include <unordered_map>
#include <vector>
#include <sstream>

struct PurchaseList {
    bool enabled = false;
    bool onlyDuringFreezeTime = false;
    bool showPrices = false;
    bool noTitleBar = false;

    enum Mode {
        Details = 0,
        Summary
    };
    int mode = Details;

    ImVec2 pos;
    ImVec2 size{ 200.0f, 200.0f };
};

struct ObserverList {
    bool enabled = false;
    bool noTitleBar = false;
    ImVec2 pos;
    ImVec2 size{ 200.0f, 200.0f };
};

struct OverlayWindow {
    OverlayWindow() = default;
    OverlayWindow(const char* windowName) : name{ windowName } {}
    bool enabled = false;
    const char* name = "";
    ImVec2 pos;
};

struct OffscreenEnemies {
    bool enabled = false;
    bool audibleOnly = false;
    bool spottedOnly = false;
};

struct {
    ColorToggleThickness reloadProgress{ 5.0f };
    ColorToggleThickness recoilCrosshair;
    ColorToggleThickness noscopeCrosshair;
    PurchaseList purchaseList;
    ObserverList observerList;
    bool ignoreFlashbang = false;
    OverlayWindow fpsCounter{ "FPS Counter" };
    OffscreenEnemies offscreenEnemies;

    ColorToggleThickness rainbowBar{ 3.0f };
    bool rainbowUp{ true };
    bool rainbowBottom{ false };
    bool rainbowLeft{ false };
    bool rainbowRight{ false };
    float rainbowScale{ 0.125f };
    bool rainbowPulse{ false };
    float rainbowPulseSpeed{ 1.0f };

    ColorToggle bombTimer{ 1.f, 0.55f, 0.f, 1.f };

    ColorToggleThickness hitMarker;
    float hitMarkerLength{ 10.f };
    float hitMarkerTime{ 0.6f };

    ColorToggle hitMarkerDamageIndicator;
    bool hitMarkerDamageIndicatorCustomize{ false };
    int hitMarkerDamageIndicatorDist{ 50 };
    float hitMarkerDamageIndicatorRatio{ 0.6f };
} miscConfig;

void Misc::drawReloadProgress(ImDrawList* drawList) noexcept
{
    if (!miscConfig.reloadProgress.enabled)
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
        const ImU32 color = Helpers::calculateColor(miscConfig.reloadProgress);
        drawList->PathStroke(color & 0xFF000000, false, miscConfig.reloadProgress.thickness);
        drawList->PathArcTo(ImGui::GetIO().DisplaySize / 2.0f, 20.0f, min, max, segments);
        drawList->PathStroke(color, false, miscConfig.reloadProgress.thickness);
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
    if (!miscConfig.recoilCrosshair.enabled)
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

    drawCrosshair(drawList, pos, Helpers::calculateColor(miscConfig.recoilCrosshair), miscConfig.recoilCrosshair.thickness);
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

                    const std::string weapon = interfaces->localize->findAsUTF8(definition->getItemBaseName());
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
        if (!miscConfig.purchaseList.enabled)
            return;

        static const auto mp_buytime = interfaces->cvar->findVar("mp_buytime");

        if ((!interfaces->engine->isInGame() || (freezeEnd != 0.0f && memory->globalVars->realtime > freezeEnd + (!miscConfig.purchaseList.onlyDuringFreezeTime ? mp_buytime->getFloat() : 0.0f)) || playerPurchases.empty() || purchaseTotal.empty()) && !gui->isOpen())
            return;
        
        if (miscConfig.purchaseList.pos != ImVec2{}) {
            ImGui::SetNextWindowPos(miscConfig.purchaseList.pos);
            miscConfig.purchaseList.pos = {};
        }

        if (miscConfig.purchaseList.size != ImVec2{}) {
            ImGui::SetNextWindowSize(ImClamp(miscConfig.purchaseList.size, {}, ImGui::GetIO().DisplaySize));
            miscConfig.purchaseList.size = {};
        }

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
        if (!gui->isOpen())
            windowFlags |= ImGuiWindowFlags_NoInputs;
        if (miscConfig.purchaseList.noTitleBar)
            windowFlags |= ImGuiWindowFlags_NoTitleBar;

        ImGui::Begin("Purchases", nullptr, windowFlags);
        ImGui::PushFont(gui->getUnicodeFont());

        if (miscConfig.purchaseList.mode == PurchaseList::Details) {
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
                    if (miscConfig.purchaseList.showPrices)
                        ImGui::TextWrapped("%s $%d: %s", it->name, purchases.totalCost, s.c_str());
                    else
                        ImGui::TextWrapped("%s: %s", it->name, s.c_str());
                }
            }
        } else if (miscConfig.purchaseList.mode == PurchaseList::Summary) {
            for (const auto& purchase : purchaseTotal)
                ImGui::TextWrapped("%dx %s", purchase.second, purchase.first.c_str());

            if (miscConfig.purchaseList.showPrices && totalCost > 0) {
                ImGui::Separator();
                ImGui::TextWrapped("Total: $%d", totalCost);
            }
        }

        ImGui::PopFont();
        ImGui::End();
    }
}

void Misc::drawObserverList() noexcept
{
    if (!miscConfig.observerList.enabled)
        return;

    GameData::Lock lock;

    const auto& observers = GameData::observers();

    if (std::none_of(observers.begin(), observers.end(), [](const auto& obs) { return obs.targetIsLocalPlayer; }) && !gui->isOpen())
        return;

    if (miscConfig.observerList.pos != ImVec2{}) {
        ImGui::SetNextWindowPos(miscConfig.observerList.pos);
        miscConfig.observerList.pos = {};
    }

    if (miscConfig.observerList.size != ImVec2{}) {
        ImGui::SetNextWindowSize(ImClamp(miscConfig.observerList.size, {}, ImGui::GetIO().DisplaySize));
        miscConfig.observerList.size = {};
    }

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
    if (!gui->isOpen())
        windowFlags |= ImGuiWindowFlags_NoInputs;
    if (miscConfig.observerList.noTitleBar)
        windowFlags |= ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Observer List", nullptr, windowFlags);
    ImGui::PushFont(gui->getUnicodeFont());

    for (const auto& observer : observers) {
        if (!observer.targetIsLocalPlayer)
            continue;

        if (const auto it = std::find_if(GameData::players().cbegin(), GameData::players().cend(), [userId = observer.playerUserId](const auto& playerData) { return playerData.userId == userId; }); it != GameData::players().cend()) {
            ImGui::TextWrapped("%s", it->name);
        }
    }

    ImGui::PopFont();
    ImGui::End();
}

void Misc::drawNoscopeCrosshair(ImDrawList* drawList) noexcept
{
    if (!miscConfig.noscopeCrosshair.enabled)
        return;

    GameData::Lock lock;
    const auto& localPlayerData = GameData::local();

    if (!localPlayerData.exists || !localPlayerData.alive)
        return;

    if (!localPlayerData.noScope)
        return;

    drawCrosshair(drawList, ImGui::GetIO().DisplaySize / 2, Helpers::calculateColor(miscConfig.noscopeCrosshair), miscConfig.noscopeCrosshair.thickness);
}

void Misc::drawFpsCounter() noexcept
{
    if (!miscConfig.fpsCounter.enabled)
        return;

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
    if (!gui->isOpen())
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
    if (!miscConfig.offscreenEnemies.enabled)
        return;

    GameData::Lock lock;

    const auto yaw = Helpers::deg2rad(interfaces->engine->getViewAngles().y);

    for (auto& player : GameData::players()) {
        if ((player.dormant && Helpers::fadingAlpha(player.fadingEndTime) == 0.0f) || !player.alive || !player.enemy || player.inViewFrustum)
            continue;

        if ((miscConfig.offscreenEnemies.audibleOnly && !player.audible && !miscConfig.offscreenEnemies.spottedOnly)
            || (miscConfig.offscreenEnemies.spottedOnly && !player.spotted && !(miscConfig.offscreenEnemies.audibleOnly && player.audible))) // if both "Audible Only" and "Spotted Only" are on treat them as audible OR spotted
            return;

        const auto positionDiff = GameData::local().origin - player.origin;

        auto x = std::cos(yaw) * positionDiff.y - std::sin(yaw) * positionDiff.x;
        auto y = std::cos(yaw) * positionDiff.x + std::sin(yaw) * positionDiff.y;
        if (const auto len = std::sqrt(x * x + y * y); len != 0.0f) {
            x /= len;
            y /= len;
        }

        const auto pos = ImGui::GetIO().DisplaySize / 2 + ImVec2{ x, y } * 200;
        if (player.fadingEndTime != 0.0f)
            Helpers::setAlphaFactor(Helpers::fadingAlpha(player.fadingEndTime));
        const auto color = Helpers::calculateColor(255, 255, 255, 255);
        Helpers::setAlphaFactor(1.0f);

        constexpr float avatarRadius = 13.0f;

        drawList->AddCircleFilled(pos, avatarRadius + 1, color & IM_COL32_A_MASK, 40);

        const auto texture = player.getAvatarTexture();

        const bool pushTextureId = drawList->_TextureIdStack.empty() || texture != drawList->_TextureIdStack.back();
        if (pushTextureId)
            drawList->PushTextureID(texture);

        const int vertStartIdx = drawList->VtxBuffer.Size;
        drawList->AddCircleFilled(pos, avatarRadius, color, 40);
        const int vertEndIdx = drawList->VtxBuffer.Size;
        ImGui::ShadeVertsLinearUV(drawList, vertStartIdx, vertEndIdx, pos - ImVec2{ avatarRadius, avatarRadius }, pos + ImVec2{ avatarRadius, avatarRadius }, { 0, 0 }, { 1, 1 }, true);

        if (pushTextureId)
            drawList->PopTextureID();
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
    if (!miscConfig.rainbowBar.enabled)
        return;

    float colorR = 0;
    float colorG = 0;
    float colorB = 0;
    if (miscConfig.rainbowBar.rainbow) {
        colorR = std::sin(miscConfig.rainbowBar.rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f;
        colorG = std::sin(miscConfig.rainbowBar.rainbowSpeed * memory->globalVars->realtime + 2 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f;
        colorB = std::sin(miscConfig.rainbowBar.rainbowSpeed * memory->globalVars->realtime + 4 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f;
    }
    else {
        colorR = miscConfig.rainbowBar.color[0];
        colorG = miscConfig.rainbowBar.color[1];
        colorB = miscConfig.rainbowBar.color[2];
    }
    float colorA = miscConfig.rainbowBar.color[3];
    float tickness = miscConfig.rainbowBar.thickness;
    float scale = miscConfig.rainbowScale;
    float pulse, pulseAlpha;
    if (miscConfig.rainbowPulse) {
        pulse = std::sin(miscConfig.rainbowPulseSpeed * memory->globalVars->realtime) * 0.5f + 0.5f;
        pulseAlpha = (std::sin(miscConfig.rainbowPulseSpeed * memory->globalVars->realtime) * 0.5f + 0.5f) * colorA;
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
        miscConfig.rainbowBar.thickness = ds.y;
        tickness = ds.y;
    }

    //drawList->AddRectFilledMultiColor(upper - left, lower - right, Color Upper Left, Color Upper Right, Color Bottom Right, Color Bottom Left);

    if (miscConfig.rainbowBottom) {
        // Bottom
        drawList->AddRectFilledMultiColor({ zero.x, ds.y - tickness }, { ds.x / 2, ds.y }, indigo0, blue0, blue, indigo);
        drawList->AddRectFilledMultiColor({ ds.x / 2, ds.y - tickness }, { ds.x, ds.y }, blue0, cyan0, cyan, blue);
    }
    if (miscConfig.rainbowLeft) {
        // Left
        drawList->AddRectFilledMultiColor(zero, { tickness, ds.y / 2 }, red, red0, magenta0, magenta);
        drawList->AddRectFilledMultiColor({ zero.x, ds.y / 2 }, { tickness, ds.y }, magenta, magenta0, indigo0, indigo);
    }
    if (miscConfig.rainbowRight) {
        // Right
        drawList->AddRectFilledMultiColor({ ds.x - tickness, zero.y }, { ds.x, ds.y / 2 }, chartreuse0, chartreuse, malachite, malachite0);
        drawList->AddRectFilledMultiColor({ ds.x - tickness, ds.y / 2 }, ds, malachite0, malachite, cyan, cyan0);
    }
    if (miscConfig.rainbowUp) {
        // Upper
        drawList->AddRectFilledMultiColor(zero, { ds.x / 2, tickness + (0.0f) }, red, amber, amber0, red0);
        drawList->AddRectFilledMultiColor({ ds.x / 2, zero.y }, { ds.x, tickness + (0.0f) }, amber, chartreuse, chartreuse0, amber0);
    }
}

void Misc::drawBombTimer() noexcept
{
    if (miscConfig.bombTimer.enabled) {
        ImGui::PushFont(gui->getUnicodeFont());
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
                Helpers::calculateColor(miscConfig.bombTimer));

            if (entity->bombDefuser() != -1) {
                if (PlayerInfo playerInfo; interfaces->engine->getPlayerInfo(interfaces->entityList->getEntityFromHandle(entity->bombDefuser())->index(), playerInfo)) {
                    drawPositionY += ImGui::CalcTextSize(" ").y;

                    std::stringstream defusingText;
                    defusingText << playerInfo.name << " is defusing: " <<
                        std::fixed << std::showpoint << std::setprecision(3) <<
                        (std::max)(entity->defuseCountDown() - memory->globalVars->currenttime, 0.0f) << " s";

                    drawList->AddText({ (ImGui::GetIO().DisplaySize.x - ImGui::CalcTextSize(defusingText.str().c_str()).x) / 2, drawPositionY },
                        Helpers::calculateColor(miscConfig.bombTimer),
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
        ImGui::PopFont();
    }
}

void Misc::hitMarker(GameEvent* event) noexcept
{
    if (!miscConfig.hitMarker.enabled || !localPlayer)
        return;

    static float lastHitTime = 0.0f;

    if (event && interfaces->engine->getPlayerForUserId(event->getInt("attacker")) == localPlayer->index()) {
        lastHitTime = memory->globalVars->realtime;
        return;
    }

    if (lastHitTime + miscConfig.hitMarkerTime < memory->globalVars->realtime)
        return;

    const auto ds = ImGui::GetIO().DisplaySize;

    auto start = 4;
    const auto width_mid = ds.x / 2;
    const auto height_mid = ds.y / 2;

    auto drawList = ImGui::GetBackgroundDrawList();
    ImU32 color = Helpers::calculateColor(miscConfig.hitMarker);
    drawList->AddLine({ width_mid + miscConfig.hitMarkerLength, height_mid + miscConfig.hitMarkerLength }, { width_mid + start, height_mid + start }, color, miscConfig.hitMarker.thickness);
    drawList->AddLine({ width_mid - miscConfig.hitMarkerLength, height_mid + miscConfig.hitMarkerLength }, { width_mid - start, height_mid + start }, color, miscConfig.hitMarker.thickness);
    drawList->AddLine({ width_mid + miscConfig.hitMarkerLength, height_mid - miscConfig.hitMarkerLength }, { width_mid + start, height_mid - start }, color, miscConfig.hitMarker.thickness);
    drawList->AddLine({ width_mid - miscConfig.hitMarkerLength, height_mid - miscConfig.hitMarkerLength }, { width_mid - start, height_mid - start }, color, miscConfig.hitMarker.thickness);
}

struct HitMarkerInfo {
    float hitMarkerExpTime;
    int hitMarkerDmg;
};

std::vector<HitMarkerInfo> hitMarkerInfo;

void Misc::hitMarkerSetDamageIndicator(GameEvent* event) noexcept
{
    if (!localPlayer)
        return;

    if (miscConfig.hitMarkerDamageIndicator.enabled)
        if (event && interfaces->engine->getPlayerForUserId(event->getInt("attacker")) == localPlayer->index())
            hitMarkerInfo.push_back({ memory->globalVars->realtime + miscConfig.hitMarkerTime, event->getInt("dmg_health") });
}

void Misc::hitMarkerDamageIndicator() noexcept
{
    if (miscConfig.hitMarkerDamageIndicator.enabled) {
        if (hitMarkerInfo.empty()) return;

        const auto ds = ImGui::GetIO().DisplaySize;

        ImGui::PushFont(gui->getUnicodeFont());
        for (size_t i = 0; i < hitMarkerInfo.size(); i++) {
            const auto diff = hitMarkerInfo.at(i).hitMarkerExpTime - memory->globalVars->realtime;

            if (diff < 0.f) {
                hitMarkerInfo.erase(hitMarkerInfo.begin() + i);
                continue;
            }

            const auto dist = miscConfig.hitMarkerDamageIndicatorCustomize ? miscConfig.hitMarkerDamageIndicatorDist : 50;
            const auto ratio = (miscConfig.hitMarkerDamageIndicatorCustomize ? miscConfig.hitMarkerDamageIndicatorRatio : 0.6f) - diff;

            auto drawList = ImGui::GetBackgroundDrawList();
            ImU32 color = Helpers::calculateColor(miscConfig.hitMarkerDamageIndicator);
            drawList->AddText({ ds.x / 2 + (miscConfig.hitMarker.enabled ? miscConfig.hitMarkerLength + 2 : 2) + ratio * dist / 2, ds.y / 2 + (miscConfig.hitMarker.enabled ? miscConfig.hitMarkerLength + 2 : 2) + ratio * dist }, color, std::to_string(hitMarkerInfo.at(i).hitMarkerDmg).c_str());
        }
        ImGui::PopFont();
    }
}

void Misc::draw(ImDrawList* drawList) noexcept
{
    drawReloadProgress(drawList);
    drawRecoilCrosshair(drawList);
    purchaseList();
    drawObserverList();
    drawNoscopeCrosshair(drawList);
    drawFpsCounter();
    drawOffscreenEnemies(drawList);

    rainbowBar(drawList);
    drawBombTimer();
    hitMarker();
    hitMarkerDamageIndicator();
}

void Misc::drawGUI() noexcept
{
    ImGuiCustom::colorPicker("Reload Progress", miscConfig.reloadProgress);
    ImGuiCustom::colorPicker("Recoil Crosshair", miscConfig.recoilCrosshair);
    ImGuiCustom::colorPicker("Noscope Crosshair", miscConfig.noscopeCrosshair);
    ImGui::Checkbox("Purchase List", &miscConfig.purchaseList.enabled);
    ImGui::SameLine();

    ImGui::PushID("Purchase List");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::SetNextItemWidth(75.0f);
        ImGui::Combo("Mode", &miscConfig.purchaseList.mode, "Details\0Summary\0");
        ImGui::Checkbox("Only During Freeze Time", &miscConfig.purchaseList.onlyDuringFreezeTime);
        ImGui::Checkbox("Show Prices", &miscConfig.purchaseList.showPrices);
        ImGui::Checkbox("No Title Bar", &miscConfig.purchaseList.noTitleBar);
        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::PushID("Observer List");
    ImGui::Checkbox("Observer List", &miscConfig.observerList.enabled);
    ImGui::SameLine();

    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::Checkbox("No Title Bar", &miscConfig.observerList.noTitleBar);
        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::Checkbox("Ignore Flashbang", &miscConfig.ignoreFlashbang);
    ImGui::Checkbox("FPS Counter", &miscConfig.fpsCounter.enabled);
    ImGui::Checkbox("Offscreen Enemies", &miscConfig.offscreenEnemies.enabled);
    if (miscConfig.offscreenEnemies.enabled) {
        ImGui::SameLine();
        ImGui::PushID("Offscreen Enemies");
        if (ImGui::Button("..."))
            ImGui::OpenPopup("OE");

        if (ImGui::BeginPopup("OE")) {
            ImGui::Checkbox("Audible Only", &miscConfig.offscreenEnemies.audibleOnly);
            ImGui::Checkbox("Spotted Only", &miscConfig.offscreenEnemies.spottedOnly);
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
    ImGuiCustom::colorPicker("Rainbow Bar", miscConfig.rainbowBar);
    if (miscConfig.rainbowBar.enabled) {
        ImGui::SameLine();
        ImGui::PushID("Rainbow Bar");
        if (ImGui::Button("..."))
            ImGui::OpenPopup("RB");

        if (ImGui::BeginPopup("RB")) {
            ImGui::Text("Position:");
            ImGui::Checkbox("Upper", &miscConfig.rainbowUp);
            ImGui::Checkbox("Bottom", &miscConfig.rainbowBottom);
            ImGui::Checkbox("Left", &miscConfig.rainbowLeft);
            ImGui::Checkbox("Right", &miscConfig.rainbowRight);
            ImGui::Text("Scale:");
            ImGui::SliderFloat("Scale", &miscConfig.rainbowScale, 0.03125f, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic);
            ImGui::Text("Scale presets:");
            if (ImGui::Button("0.25x"))
                miscConfig.rainbowScale = 0.03125f;
            ImGui::SameLine();
            if (ImGui::Button("0.5x"))
                miscConfig.rainbowScale = 0.0625f;
            ImGui::SameLine();
            if (ImGui::Button("1x"))
                miscConfig.rainbowScale = 0.125f;
            ImGui::SameLine();
            if (ImGui::Button("2x"))
                miscConfig.rainbowScale = 0.25f;
            ImGui::SameLine();
            if (ImGui::Button("4x"))
                miscConfig.rainbowScale = 0.5f;
            ImGui::SameLine();
            if (ImGui::Button("8x"))
                miscConfig.rainbowScale = 1.0f;
            ImGui::Text("Pulse:");
            ImGui::Checkbox("Enable", &miscConfig.rainbowPulse);
            ImGui::SliderFloat("Speed", &miscConfig.rainbowPulseSpeed, 0.1f, 25.0f, "%.1f", ImGuiSliderFlags_Logarithmic);
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
    ImGuiCustom::colorPicker("Bomb timer", miscConfig.bombTimer);

    ImGuiCustom::colorPicker("Hit marker", miscConfig.hitMarker);
    miscConfig.hitMarker.thickness = std::clamp<float>(miscConfig.hitMarker.thickness, 0.f, 10.f);
    if (miscConfig.hitMarker.enabled) {
        ImGui::SameLine();
        ImGui::PushID("Hit marker");
        if (ImGui::Button("..."))
            ImGui::OpenPopup("HM");

        if (ImGui::BeginPopup("HM")) {
            float hitMarkerLength = miscConfig.hitMarkerLength + 4.f;
            if (ImGui::SliderFloat("Hit Marker Length", &hitMarkerLength, 1.f, 16.f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
                miscConfig.hitMarkerLength = hitMarkerLength - 4.f;
            ImGui::SliderFloat("Hit marker time", &miscConfig.hitMarkerTime, 0.1f, 1.5f, "%.2fs");
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
    ImGuiCustom::colorPicker("Hit marker damage indicator", miscConfig.hitMarkerDamageIndicator);
    if (miscConfig.hitMarkerDamageIndicator.enabled) {
        ImGui::SameLine();
        ImGui::PushID("Hit marker damage indicator");
        if (ImGui::Button("..."))
            ImGui::OpenPopup("HMDI");

        if (ImGui::BeginPopup("HMDI")) {
            ImGui::Checkbox("Customize Hitmarker", &miscConfig.hitMarkerDamageIndicatorCustomize);
            if (miscConfig.hitMarkerDamageIndicatorCustomize) {
                ImGui::SliderInt(" ", &miscConfig.hitMarkerDamageIndicatorDist, 1, 100, "Dist: %d");
                ImGui::PushID(1);
                ImGui::SliderFloat(" ", &miscConfig.hitMarkerDamageIndicatorRatio, 0.1f, 1.0f, "Ratio: %.2f");
                ImGui::PopID();
            };
            ImGui::EndPopup();
        }
        ImGui::PopID();
    };
}

bool Misc::ignoresFlashbang() noexcept
{
    return miscConfig.ignoreFlashbang;
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
    WRITE("Audible Only", audibleOnly);
    WRITE("Spotted Only", spottedOnly);
}

json Misc::toJSON() noexcept
{
    json j;
    to_json(j["Reload Progress"], miscConfig.reloadProgress, ColorToggleThickness{ 5.0f });

    if (miscConfig.ignoreFlashbang)
        j["Ignore Flashbang"] = miscConfig.ignoreFlashbang;

    j["Recoil Crosshair"] = miscConfig.recoilCrosshair;
    j["Noscope Crosshair"] = miscConfig.noscopeCrosshair;
    j["Purchase List"] = miscConfig.purchaseList;
    j["Observer List"] = miscConfig.observerList;
    j["FPS Counter"] = miscConfig.fpsCounter;
    j["Offscreen Enemies"] = miscConfig.offscreenEnemies;

    j["Rainbow Bar"] = miscConfig.rainbowBar;
    j["Rainbow Up"] = miscConfig.rainbowUp;
    j["Rainbow Bottom"] = miscConfig.rainbowBottom;
    j["Rainbow Left"] = miscConfig.rainbowLeft;
    j["Rainbow Right"] = miscConfig.rainbowRight;
    j["Rainbow Scale"] = miscConfig.rainbowScale;
    j["Rainbow Pulse"] = miscConfig.rainbowPulse;
    j["Rainbow Pulse Speed"] = miscConfig.rainbowPulseSpeed;

    j["Bomb timer"] = miscConfig.bombTimer;

    j["Hit Marker"] = miscConfig.hitMarker;
    j["Hit Marker Length"] = miscConfig.hitMarkerLength;
    j["Hit Marker Time"] = miscConfig.hitMarkerTime;

    j["Hit marker damage indicator"] = miscConfig.hitMarkerDamageIndicator;
    j["Hit marker damage indicator Customize"] = miscConfig.hitMarkerDamageIndicatorCustomize;
    j["Hit marker damage indicator Dist"] = miscConfig.hitMarkerDamageIndicatorDist;
    j["Hit marker damage indicator Ratio"] = miscConfig.hitMarkerDamageIndicatorRatio;

    // Save GUI Configuration
    ImGuiStyle& style = ImGui::GetStyle();
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

    return j;
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
    read(j, "Audible Only", o.audibleOnly);
    read(j, "Spotted Only", o.spottedOnly);
}

void Misc::fromJSON(const json& j) noexcept
{
    read<value_t::object>(j, "Reload Progress", miscConfig.reloadProgress);
    read<value_t::object>(j, "Recoil Crosshair", miscConfig.recoilCrosshair);
    read<value_t::object>(j, "Noscope Crosshair", miscConfig.noscopeCrosshair);
    read<value_t::object>(j, "Purchase List", miscConfig.purchaseList);
    read<value_t::object>(j, "Observer List", miscConfig.observerList);
    read(j, "Ignore Flashbang", miscConfig.ignoreFlashbang);
    read<value_t::object>(j, "FPS Counter", miscConfig.fpsCounter);
    read<value_t::object>(j, "Offscreen Enemies", miscConfig.offscreenEnemies);

    read<value_t::object>(j, "Rainbow Bar", miscConfig.rainbowBar);
    read(j, "Rainbow Up", miscConfig.rainbowUp);
    read(j, "Rainbow Bottom", miscConfig.rainbowBottom);
    read(j, "Rainbow Left", miscConfig.rainbowLeft);
    read(j, "Rainbow Right", miscConfig.rainbowRight);
    read_number(j, "Rainbow Scale", miscConfig.rainbowScale);
    read(j, "Rainbow Pulse", miscConfig.rainbowPulse);
    read_number(j, "Rainbow Pulse Speed", miscConfig.rainbowPulseSpeed);

    read<value_t::object>(j, "Bomb timer", miscConfig.bombTimer);

    read<value_t::object>(j, "Hit Marker", miscConfig.hitMarker);
    read_number(j, "Hit Marker Length", miscConfig.hitMarkerLength);
    read_number(j, "Hit Marker Time", miscConfig.hitMarkerTime);

    read<value_t::object>(j, "Hit marker damage indicator", miscConfig.hitMarkerDamageIndicator);
    read(j, "Hit marker damage indicator Customize", miscConfig.hitMarkerDamageIndicatorCustomize);
    read_number(j, "Hit marker damage indicator Dist", miscConfig.hitMarkerDamageIndicatorDist);
    read_number(j, "Hit marker damage indicator Ratio", miscConfig.hitMarkerDamageIndicatorRatio);

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
