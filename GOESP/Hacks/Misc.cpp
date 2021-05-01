#include <algorithm>
#include <array>
#ifndef __APPLE__
#include <numbers>
#endif
#include <numeric>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "Misc.h"

#include "../imgui/imgui.h"

#include "../fnv.h"
#include "../GameData.h"
#include "../GUI.h"
#include "../Helpers.h"
#include "../Hooks.h"
#include "../Interfaces.h"
#include "../Memory.h"
#include "../PostProcessing.h"
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
    ImVec2 size{ 300.0f, 200.0f };
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

struct OffscreenEnemies : public Color {
    OffscreenEnemies() : Color{ 1.0f, 0.26f, 0.21f, 1.0f } {}
    bool enabled = false;
    HealthBar healthBar;
    bool audibleOnly = false;
    bool spottedOnly = false;
};

struct PlayerList {
    bool enabled = false;
    bool avatar = false;
    bool steamID = false;
    bool rank = false;
    bool wins = false;
    bool money = true;
    bool health = true;
    bool armor = false;
    bool lastPlace = false;

    ImVec2 pos;
    ImVec2 size{ 270.0f, 200.0f };
};

struct HitEffect {
    bool enabled = false;
};

struct HitMarker {
    ColorToggleThickness hitMarker;
    float hitMarkerStart{ 4.f };
    float hitMarkerEnd{ 10.f };
    float hitMarkerTime{ 0.6f };
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
    PlayerList playerList;
    ColorToggle molotovHull{ 1.0f, 0.27f, 0.0f, 0.3f };
    ColorToggle bombTimer{ 1.0f, 0.55f, 0.0f, 1.0f };
    ColorToggle smokeHull{ 0.0f, 0.81f, 1.0f, 0.60f };
    ColorToggle nadeBlast{ 1.0f, 0.0f, 0.09f, 0.51f };
    HitEffect hitEffect;
    
    ImGuiKey panicKey{ -1 };
    HitMarker hitMarker;
} miscConfig;

static void drawReloadProgress(ImDrawList* drawList) noexcept
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
#ifndef __APPLE__
        constexpr float pi = std::numbers::pi_v<float>;
#else
        constexpr float pi = static_cast<float>(M_PI);
#endif
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

static void drawCrosshair(ImDrawList* drawList, const ImVec2& pos, ImU32 color) noexcept
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

static void drawRecoilCrosshair(ImDrawList* drawList) noexcept
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

    drawCrosshair(drawList, pos, Helpers::calculateColor(miscConfig.recoilCrosshair));
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
            const auto player = Entity::asPlayer(interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserId(event->getInt("userid"))));
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
            if (ImGui::BeginTable("table", 3, ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable)) {
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 100.0f);
                ImGui::TableSetupColumn("Price", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize);
                ImGui::TableSetupColumn("Purchases", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetColumnEnabled(1, miscConfig.purchaseList.showPrices);

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

                    ImGui::TableNextRow();

#ifndef __APPLE__
                    const auto it = std::ranges::find(GameData::players(), userId, &PlayerData::userId);
#else
                    const auto it = [&, userId = userId] {
                        for (auto its = GameData::players().cbegin(); its != GameData::players().cend(); ++its)
                            if (its->userId == userId)
                                return its;
                        return GameData::players().cend();
                    }();
#endif
                	
                    if (it != GameData::players().cend()) {
                        if (ImGui::TableNextColumn())
                            ImGui::textEllipsisInTableCell(it->name.c_str());
                        if (ImGui::TableNextColumn())
                            ImGui::TextColored({ 0.0f, 1.0f, 0.0f, 1.0f }, "$%d", purchases.totalCost);
                        if (ImGui::TableNextColumn())
                            ImGui::TextWrapped("%s", s.c_str());
                    }
                }

                ImGui::EndTable();
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

static void drawObserverList() noexcept
{
    if (!miscConfig.observerList.enabled)
        return;

    GameData::Lock lock;

    const auto& observers = GameData::observers();

#ifndef __APPLE__
    if (std::ranges::none_of(observers, [](const auto& obs) { return obs.targetIsLocalPlayer; }) && !gui->isOpen())
#else
    if (std::none_of(observers.begin(), observers.end(), [](const auto& obs) { return obs.targetIsLocalPlayer; }) && !gui->isOpen())
#endif
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

#ifndef __APPLE__
        const auto it = std::ranges::find(GameData::players(), observer.playerUserId, &PlayerData::userId);
#else
        const auto it = [&] {
            for (auto its = GameData::players().cbegin(); its != GameData::players().cend(); ++its)
                if (its->userId == observer.playerUserId)
                    return its;
            return GameData::players().cend();
        }();
#endif
    	
        if (it != GameData::players().cend()) {
            ImGui::TextUnformatted(it->name.c_str());
        }
    }

    ImGui::PopFont();
    ImGui::End();
}

static void drawNoscopeCrosshair(ImDrawList* drawList) noexcept
{
    if (!miscConfig.noscopeCrosshair.enabled)
        return;

    {
        GameData::Lock lock;
        if (const auto& local = GameData::local(); !local.exists || !local.alive || !local.noScope)
            return;
    }

    drawCrosshair(drawList, ImGui::GetIO().DisplaySize / 2, Helpers::calculateColor(miscConfig.noscopeCrosshair));
}

static void drawFpsCounter() noexcept
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

// ImGui::ShadeVertsLinearColorGradientKeepAlpha() modified to do interpolation in HSV
static void shadeVertsHSVColorGradientKeepAlpha(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1)
{
    ImVec2 gradient_extent = gradient_p1 - gradient_p0;
    float gradient_inv_length2 = 1.0f / ImLengthSqr(gradient_extent);
    ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
    ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;

    ImVec4 col0HSV = ImGui::ColorConvertU32ToFloat4(col0);
    ImVec4 col1HSV = ImGui::ColorConvertU32ToFloat4(col1);
    ImGui::ColorConvertRGBtoHSV(col0HSV.x, col0HSV.y, col0HSV.z, col0HSV.x, col0HSV.y, col0HSV.z);
    ImGui::ColorConvertRGBtoHSV(col1HSV.x, col1HSV.y, col1HSV.z, col1HSV.x, col1HSV.y, col1HSV.z);
    ImVec4 colDelta = col1HSV - col0HSV;

    for (ImDrawVert* vert = vert_start; vert < vert_end; vert++)
    {
        float d = ImDot(vert->pos - gradient_p0, gradient_extent);
        float t = ImClamp(d * gradient_inv_length2, 0.0f, 1.0f);

        float h = col0HSV.x + colDelta.x * t;
        float s = col0HSV.y + colDelta.y * t;
        float v = col0HSV.z + colDelta.z * t;

        ImVec4 rgb;
        ImGui::ColorConvertHSVtoRGB(h, s, v, rgb.x, rgb.y, rgb.z);
        vert->col = (ImGui::ColorConvertFloat4ToU32(rgb) & ~IM_COL32_A_MASK) | (vert->col & IM_COL32_A_MASK);
    }
}

static void drawOffscreenEnemies(ImDrawList* drawList) noexcept
{
    if (!miscConfig.offscreenEnemies.enabled)
        return;

    const auto yaw = Helpers::deg2rad(interfaces->engine->getViewAngles().y);

    GameData::Lock lock;
    for (auto& player : GameData::players()) {
        if ((player.dormant && player.fadingAlpha() == 0.0f) || !player.alive || !player.enemy || player.inViewFrustum)
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

        Helpers::setAlphaFactor(player.fadingAlpha());
        const auto white = Helpers::calculateColor(255, 255, 255, 255);
        const auto background = Helpers::calculateColor(0, 0, 0, 80);
        const auto triangleColor = Helpers::calculateColor(miscConfig.offscreenEnemies);
        const auto healthBarColor = miscConfig.offscreenEnemies.healthBar.type == HealthBar::HealthBased ? Helpers::healthColor(std::clamp(player.health / 100.0f, 0.0f, 1.0f)) : Helpers::calculateColor(miscConfig.offscreenEnemies.healthBar);
        Helpers::setAlphaFactor(1.0f);

        constexpr auto avatarRadius = 13.0f;
        constexpr auto triangleSize = 10.0f;

        const auto pos = ImGui::GetIO().DisplaySize / 2 + ImVec2{ x, y } * 200;
        const auto trianglePos = pos + ImVec2{ x, y } * (avatarRadius + (miscConfig.offscreenEnemies.healthBar.enabled ? 5 : 3));

        const ImVec2 trianglePoints[]{
            trianglePos + ImVec2{  0.4f * y, -0.4f * x } * triangleSize,
            trianglePos + ImVec2{  1.0f * x,  1.0f * y } * triangleSize,
            trianglePos + ImVec2{ -0.4f * y,  0.4f * x } * triangleSize
        };

        drawList->AddConvexPolyFilled(trianglePoints, 3, triangleColor);
        drawList->AddCircleFilled(pos, avatarRadius + 1, white & IM_COL32_A_MASK, 40);

        const auto texture = player.getAvatarTexture();

        const bool pushTextureId = drawList->_TextureIdStack.empty() || texture != drawList->_TextureIdStack.back();
        if (pushTextureId)
            drawList->PushTextureID(texture);

        const int vertStartIdx = drawList->VtxBuffer.Size;
        drawList->AddCircleFilled(pos, avatarRadius, white, 40);
        const int vertEndIdx = drawList->VtxBuffer.Size;
        ImGui::ShadeVertsLinearUV(drawList, vertStartIdx, vertEndIdx, pos - ImVec2{ avatarRadius, avatarRadius }, pos + ImVec2{ avatarRadius, avatarRadius }, { 0, 0 }, { 1, 1 }, true);

        if (pushTextureId)
            drawList->PopTextureID();

        if (miscConfig.offscreenEnemies.healthBar.enabled) {
            const auto radius = avatarRadius + 2;
            const auto healthFraction = std::clamp(player.health / 100.0f, 0.0f, 1.0f);

            drawList->AddCircle(pos, radius, background, 40, 3.0f);

            const int vertStartIdx = drawList->VtxBuffer.Size;
            if (healthFraction == 1.0f) { // sometimes PathArcTo is missing one top pixel when drawing a full circle, so draw it with AddCircle
                drawList->AddCircle(pos, radius, healthBarColor, 40, 2.0f);
            } else {
#ifndef __APPLE__
                constexpr float pi = std::numbers::pi_v<float>;
#else
                constexpr float pi = static_cast<float>(M_PI);
#endif
                drawList->PathArcTo(pos, radius - 0.5f, pi / 2 - pi * healthFraction, pi / 2 + pi * healthFraction, 40);
                drawList->PathStroke(healthBarColor, false, 2.0f);
            }
            const int vertEndIdx = drawList->VtxBuffer.Size;

            if (miscConfig.offscreenEnemies.healthBar.type == HealthBar::Gradient)
                shadeVertsHSVColorGradientKeepAlpha(drawList, vertStartIdx, vertEndIdx, pos - ImVec2{ 0.0f, radius }, pos + ImVec2{ 0.0f, radius }, IM_COL32(0, 255, 0, 255), IM_COL32(255, 0, 0, 255));
        }
    }
}

static void drawBombTimer() noexcept
{
    if (!miscConfig.bombTimer.enabled)
        return;

    GameData::Lock lock;

    const auto& plantedC4 = GameData::plantedC4();
    if (plantedC4.blowTime == 0.0f && !gui->isOpen())
        return;

    if (!gui->isOpen()) {
        ImGui::SetNextWindowBgAlpha(0.3f);
    }

    static float windowWidth = 200.0f;
    ImGui::SetNextWindowPos({ (ImGui::GetIO().DisplaySize.x - 200.0f) / 2.0f, 60.0f }, ImGuiCond_Once);
    ImGui::SetNextWindowSize({ windowWidth, 0 }, ImGuiCond_Once);

    if (!gui->isOpen())
        ImGui::SetNextWindowSize({ windowWidth, 0 });

    ImGui::SetNextWindowSizeConstraints({ 0, -1 }, { FLT_MAX, -1 });
    ImGui::Begin("Bomb Timer", nullptr, ImGuiWindowFlags_NoTitleBar | (gui->isOpen() ? 0 : ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDecoration));

    std::ostringstream ss; ss << "Bomb on " << (!plantedC4.bombsite ? 'A' : 'B') << " : " << std::fixed << std::showpoint << std::setprecision(3) << std::max(plantedC4.blowTime - memory->globalVars->currenttime, 0.0f) << " s";

    ImGui::textUnformattedCentered(ss.str().c_str());

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Helpers::calculateColor(miscConfig.bombTimer, true));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f });
    ImGui::progressBarFullWidth((plantedC4.blowTime - memory->globalVars->currenttime) / plantedC4.timerLength, 5.0f);

    if (plantedC4.defuserHandle != -1) {
        const bool canDefuse = plantedC4.blowTime >= plantedC4.defuseCountDown;

        if (plantedC4.defuserHandle == GameData::local().handle) {
            if (canDefuse) {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
                ImGui::textUnformattedCentered("You can defuse!");
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
                ImGui::textUnformattedCentered("You can not defuse!");
            }
            ImGui::PopStyleColor();
        } else if (const auto defusingPlayer = GameData::playerByHandle(plantedC4.defuserHandle)) {
            std::ostringstream ss; ss << defusingPlayer->name << " is defusing: " << std::fixed << std::showpoint << std::setprecision(3) << std::max(plantedC4.defuseCountDown - memory->globalVars->currenttime, 0.0f) << " s";

            ImGui::textUnformattedCentered(ss.str().c_str());

            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, canDefuse ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255));
            ImGui::progressBarFullWidth((plantedC4.defuseCountDown - memory->globalVars->currenttime) / plantedC4.defuseLength, 5.0f);
            ImGui::PopStyleColor();
        }
    }

    windowWidth = ImGui::GetCurrentWindow()->SizeFull.x;

    ImGui::PopStyleColor(2);
    ImGui::End();
}

void Misc::drawGUI() noexcept
{
    if (!ImGui::BeginTable("table", 2))
        return;

    ImGui::TableNextColumn();

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
    ImGuiCustom::colorPicker("Offscreen Enemies", miscConfig.offscreenEnemies, &miscConfig.offscreenEnemies.enabled);
    ImGui::SameLine();
    ImGui::PushID("Offscreen Enemies");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::Checkbox("Health Bar", &miscConfig.offscreenEnemies.healthBar.enabled);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.0f);
        ImGui::Combo("Type", &miscConfig.offscreenEnemies.healthBar.type, "Gradient\0Solid\0Health-based\0");
        if (miscConfig.offscreenEnemies.healthBar.type == HealthBar::Solid) {
            ImGui::SameLine();
            ImGuiCustom::colorPicker("", static_cast<Color&>(miscConfig.offscreenEnemies.healthBar));
        }
        ImGui::Checkbox("Audible Only", &miscConfig.offscreenEnemies.audibleOnly);
        ImGui::Checkbox("Spotted Only", &miscConfig.offscreenEnemies.spottedOnly);
        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::PushID("Player List");
    ImGui::Checkbox("Player List", &miscConfig.playerList.enabled);
    ImGui::SameLine();

    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::Checkbox("Avatar", &miscConfig.playerList.avatar);
        ImGui::Checkbox("Steam ID", &miscConfig.playerList.steamID);
        ImGui::Checkbox("Rank", &miscConfig.playerList.rank);
        ImGui::Checkbox("Wins", &miscConfig.playerList.wins);
        ImGui::Checkbox("Money", &miscConfig.playerList.money);
        ImGui::Checkbox("Health", &miscConfig.playerList.health);
        ImGui::Checkbox("Armor", &miscConfig.playerList.armor);
        ImGui::Checkbox("Last Place", &miscConfig.playerList.lastPlace);
        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGuiCustom::colorPicker("Molotov Hull", miscConfig.molotovHull);
    ImGuiCustom::colorPicker("Bomb Timer", miscConfig.bombTimer);

    ImGui::TableNextColumn();

    ImGuiCustom::colorPicker("Smoke Hull", miscConfig.smokeHull);
    ImGuiCustom::colorPicker("Nade Blast", miscConfig.nadeBlast);
    ImGui::Checkbox("Hit Effect", &miscConfig.hitEffect.enabled);

    int panicKeyCombo = miscConfig.panicKey + 1;
    if (ImGui::Combo("Panic Key", &panicKeyCombo,
        "None\0"
        "Tab\0"
        "LeftArrow\0"
        "RightArrow\0"
        "UpArrow\0"
        "DownArrow\0"
        "PageUp\0"
        "PageDown\0"
        "Home\0"
        "End\0"
        "Insert\0"
        "Delete\0"
        "Backspace\0"
        "Space\0"
        "Enter\0"
        "Escape\0"
        "KeyPadEnter\0"
        "A\0"
        "C\0"
        "V\0"
        "X\0"
        "Y\0"
        "Z\0"))
        miscConfig.panicKey = panicKeyCombo - 1;
    	
    ImGuiCustom::colorPicker("Hit Marker", miscConfig.hitMarker.hitMarker);
    miscConfig.hitMarker.hitMarker.thickness = std::clamp<float>(miscConfig.hitMarker.hitMarker.thickness, 0.f, 10.f);
    if (miscConfig.hitMarker.hitMarker.enabled) {
        ImGui::SameLine();
        ImGui::PushID("Hit Marker");
        if (ImGui::Button("..."))
            ImGui::OpenPopup("HM");

        if (ImGui::BeginPopup("HM")) {
            ImGui::SliderFloat("Hit Marker Start", &miscConfig.hitMarker.hitMarkerStart, 1.f, 16.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            float hitMarkerLength = miscConfig.hitMarker.hitMarkerEnd + miscConfig.hitMarker.hitMarkerStart;
            if (ImGui::SliderFloat("Hit Marker End", &hitMarkerLength, 1.f, 16.f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
                miscConfig.hitMarker.hitMarkerEnd = hitMarkerLength - miscConfig.hitMarker.hitMarkerStart;
            ImGui::SliderFloat("Hit Marker Time", &miscConfig.hitMarker.hitMarkerTime, 0.1f, 1.5f, "%.2fs");
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    ImGui::EndTable();
}

bool Misc::ignoresFlashbang() noexcept
{
    return miscConfig.ignoreFlashbang;
}

static void drawPlayerList() noexcept
{
    if (!miscConfig.playerList.enabled)
        return;

    if (miscConfig.playerList.pos != ImVec2{}) {
        ImGui::SetNextWindowPos(miscConfig.playerList.pos);
        miscConfig.playerList.pos = {};
    }

    if (miscConfig.playerList.size != ImVec2{}) {
        ImGui::SetNextWindowSize(ImClamp(miscConfig.playerList.size, {}, ImGui::GetIO().DisplaySize));
        miscConfig.playerList.size = {};
    }

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
    if (!gui->isOpen())
        windowFlags |= ImGuiWindowFlags_NoInputs;

    GameData::Lock lock;
    if (GameData::players().empty() && !gui->isOpen())
        return;

    if (ImGui::Begin("Player List", nullptr, windowFlags)) {
        if (ImGui::beginTable("", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_Hideable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable)) {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 120.0f);
            ImGui::TableSetupColumn("Steam ID", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize);
            ImGui::TableSetupColumn("Rank", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize);
            ImGui::TableSetupColumn("Wins", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize);
            ImGui::TableSetupColumn("Money", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize);
            ImGui::TableSetupColumn("Health", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize);
            ImGui::TableSetupColumn("Armor", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize);
            ImGui::TableSetupColumn("Last Place", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize);
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetColumnEnabled(1, miscConfig.playerList.steamID);
            ImGui::TableSetColumnEnabled(2, miscConfig.playerList.rank);
            ImGui::TableSetColumnEnabled(3, miscConfig.playerList.wins);
            ImGui::TableSetColumnEnabled(4, miscConfig.playerList.money);
            ImGui::TableSetColumnEnabled(5, miscConfig.playerList.health);
            ImGui::TableSetColumnEnabled(6, miscConfig.playerList.armor);
            ImGui::TableSetColumnEnabled(7, miscConfig.playerList.lastPlace);

            ImGui::TableHeadersRow();

            std::vector<std::reference_wrapper<const PlayerData>> playersOrdered{ GameData::players().begin(), GameData::players().end() };
#ifndef __APPLE__
            std::ranges::sort(playersOrdered, [](const PlayerData& a, const PlayerData& b) {
#else
            std::sort(playersOrdered.begin(), playersOrdered.end(), [](const PlayerData& a, const PlayerData& b) {
#endif
                // enemies first
                if (a.enemy != b.enemy)
                    return a.enemy && !b.enemy;

                return a.handle < b.handle;
            });

            ImGui::PushFont(gui->getUnicodeFont());

            for (const PlayerData& player : playersOrdered) {
                ImGui::TableNextRow();
                ImGui::PushID(ImGui::TableGetRowIndex());

                if (ImGui::TableNextColumn()) {
                    if (miscConfig.playerList.avatar) {
                        ImGui::Image(player.getAvatarTexture(), { ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight() });
                        ImGui::SameLine();
                    }
                    ImGui::textEllipsisInTableCell(player.name.c_str());
                }

                if (ImGui::TableNextColumn() && ImGui::smallButtonFullWidth("Copy", player.steamID == 0))
                    ImGui::SetClipboardText(std::to_string(player.steamID).c_str());

                if (ImGui::TableNextColumn()) {
                    ImGui::Image(player.getRankTexture(), { 2.45f /* -> proportion 49x20px */ * ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight() });
                    if (ImGui::IsItemHovered()) {
                        ImGui::BeginTooltip();
                        ImGui::PushFont(nullptr);
                        ImGui::TextUnformatted(player.getRankName().c_str());
                        ImGui::PopFont();
                        ImGui::EndTooltip();
                    }
                }

                if (ImGui::TableNextColumn())
                    ImGui::Text("%d", player.competitiveWins);

                if (ImGui::TableNextColumn())
                    ImGui::TextColored({ 0.0f, 1.0f, 0.0f, 1.0f }, "$%d", player.money);

                if (ImGui::TableNextColumn()) {
                    if (!player.alive)
                        ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "%s", "DEAD");
                    else
                        ImGui::Text("%d HP", player.health);
                }

                if (ImGui::TableNextColumn())
                    ImGui::Text("%d", player.armor);

                if (ImGui::TableNextColumn())
                    ImGui::TextUnformatted(player.lastPlaceName.c_str());

                ImGui::PopID();
            }

            ImGui::PopFont();
            ImGui::EndTable();
        }
    }
    ImGui::End();
}

static void drawMolotovHull(ImDrawList* drawList) noexcept
{
    if (!miscConfig.molotovHull.enabled)
        return;

    const auto color = Helpers::calculateColor(miscConfig.molotovHull);

    static const auto flameCircumference = []{
        std::array<Vector, 72> points;
        for (std::size_t i = 0; i < points.size(); ++i) {
            constexpr auto flameRadius = 60.0f; // https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/game/server/cstrike15/Effects/inferno.cpp#L889
            points[i] = Vector{ flameRadius * std::cos(Helpers::deg2rad(i * (360.0f / points.size()))),
                                flameRadius * std::sin(Helpers::deg2rad(i * (360.0f / points.size()))),
                                0.0f };
        }
        return points;
    }();

    GameData::Lock lock;
    for (const auto& molotov : GameData::infernos()) {
        for (const auto& pos : molotov.points) {
            std::array<ImVec2, flameCircumference.size()> screenPoints;
            std::size_t count = 0;

            for (const auto& point : flameCircumference) {
                if (GameData::worldToScreen(pos + point, screenPoints[count]))
                    ++count;
            }

            if (count < 3)
                continue;

            std::swap(screenPoints[0], *std::min_element(screenPoints.begin(), screenPoints.begin() + count, [](const auto& a, const auto& b) { return a.y < b.y || (a.y == b.y && a.x < b.x); }));

            constexpr auto orientation = [](const ImVec2& a, const ImVec2& b, const ImVec2& c) {
                return (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
            };

            std::sort(screenPoints.begin() + 1, screenPoints.begin() + count, [&](const auto& a, const auto& b) { return orientation(screenPoints[0], a, b) > 0.0f; });
            drawList->AddConvexPolyFilled(screenPoints.data(), count, color);
        }
    }
}

#define IM_NORMALIZE2F_OVER_ZERO(VX,VY)     do { float d2 = VX*VX + VY*VY; if (d2 > 0.0f) { float inv_len = 1.0f / ImSqrt(d2); VX *= inv_len; VY *= inv_len; } } while (0)
#define IM_FIXNORMAL2F(VX,VY)               do { float d2 = VX*VX + VY*VY; if (d2 < 0.5f) d2 = 0.5f; float inv_lensq = 1.0f / d2; VX *= inv_lensq; VY *= inv_lensq; } while (0)

static auto generateAntialiasedDot() noexcept
{
    constexpr auto segments = 4;
    constexpr auto radius = 1.0f;

    std::array<ImVec2, segments> circleSegments;

    for (int i = 0; i < segments; ++i) {
        circleSegments[i] = ImVec2{ radius * std::cos(Helpers::deg2rad(i * (360.0f / segments) + 45.0f)),
                                    radius * std::sin(Helpers::deg2rad(i * (360.0f / segments) + 45.0f)) };
    }

    // based on ImDrawList::AddConvexPolyFilled()
    const float AA_SIZE = 1.0f; // _FringeScale;
    constexpr int idx_count = (segments - 2) * 3 + segments * 6;
    constexpr int vtx_count = (segments * 2);

    std::array<ImDrawIdx, idx_count> indices;
    std::size_t indexIdx = 0;

    // Add indexes for fill
    for (int i = 2; i < segments; ++i) {
        indices[indexIdx++] = 0;
        indices[indexIdx++] = (i - 1) << 1;
        indices[indexIdx++] = i << 1;
    }

    // Compute normals
    std::array<ImVec2, segments> temp_normals;
    for (int i0 = segments - 1, i1 = 0; i1 < segments; i0 = i1++) {
        const ImVec2& p0 = circleSegments[i0];
        const ImVec2& p1 = circleSegments[i1];
        float dx = p1.x - p0.x;
        float dy = p1.y - p0.y;
        IM_NORMALIZE2F_OVER_ZERO(dx, dy);
        temp_normals[i0].x = dy;
        temp_normals[i0].y = -dx;
    }

    std::array<ImVec2, vtx_count> vertices;
    std::size_t vertexIdx = 0;

    for (int i0 = segments - 1, i1 = 0; i1 < segments; i0 = i1++) {
        // Average normals
        const ImVec2& n0 = temp_normals[i0];
        const ImVec2& n1 = temp_normals[i1];
        float dm_x = (n0.x + n1.x) * 0.5f;
        float dm_y = (n0.y + n1.y) * 0.5f;
        IM_FIXNORMAL2F(dm_x, dm_y);
        dm_x *= AA_SIZE * 0.5f;
        dm_y *= AA_SIZE * 0.5f;

        vertices[vertexIdx++] = ImVec2{ circleSegments[i1].x - dm_x, circleSegments[i1].y - dm_y };
        vertices[vertexIdx++] = ImVec2{ circleSegments[i1].x + dm_x, circleSegments[i1].y + dm_y };

        indices[indexIdx++] = (i1 << 1);
        indices[indexIdx++] = (i0 << 1);
        indices[indexIdx++] = (i0 << 1) + 1;
        indices[indexIdx++] = (i0 << 1) + 1;
        indices[indexIdx++] = (i1 << 1) + 1;
        indices[indexIdx++] = (i1 << 1);
    }

    return std::make_pair(vertices, indices);
}

template <std::size_t N>
static auto generateSpherePoints() noexcept
{
    constexpr auto goldenAngle = static_cast<float>(2.399963229728653);

    std::array<Vector, N> points;
    for (std::size_t i = 1; i <= points.size(); ++i) {
        const auto latitude = std::asin(2.0f * i / (points.size() + 1) - 1.0f);
        const auto longitude = goldenAngle * i;

        points[i - 1] = Vector{ std::cos(longitude) * std::cos(latitude), std::sin(longitude) * std::cos(latitude), std::sin(latitude) };
    }
    return points;
};

template <std::size_t VTX_COUNT, std::size_t IDX_COUNT>
static void drawPrecomputedPrimitive(ImDrawList* drawList, const ImVec2& pos, ImU32 color, const std::array<ImVec2, VTX_COUNT>& vertices, const std::array<ImDrawIdx, IDX_COUNT>& indices) noexcept
{
    drawList->PrimReserve(indices.size(), vertices.size());

    const ImU32 colors[2]{ color, color & ~IM_COL32_A_MASK };
    const auto uv = ImGui::GetDrawListSharedData()->TexUvWhitePixel;
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        drawList->_VtxWritePtr[i].pos = vertices[i] + pos;
        drawList->_VtxWritePtr[i].uv = uv;
        drawList->_VtxWritePtr[i].col = colors[i & 1];
    }
    drawList->_VtxWritePtr += vertices.size();

    std::memcpy(drawList->_IdxWritePtr, indices.data(), indices.size() * sizeof(ImDrawIdx));

    const auto baseIdx = drawList->_VtxCurrentIdx;
    for (std::size_t i = 0; i < indices.size(); ++i)
        drawList->_IdxWritePtr[i] += baseIdx;

    drawList->_IdxWritePtr += indices.size();
    drawList->_VtxCurrentIdx += vertices.size();
}

static void drawSmokeHull(ImDrawList* drawList) noexcept
{
    if (!miscConfig.smokeHull.enabled)
        return;

    const auto color = Helpers::calculateColor(miscConfig.smokeHull);

    static const auto spherePoints = generateSpherePoints<2000>();
    static const auto [vertices, indices] = generateAntialiasedDot();

    constexpr auto animationDuration = 0.35f;

    GameData::Lock lock;
    for (const auto& smoke : GameData::smokes()) {
        for (const auto& point : spherePoints) {
            const auto radius = ImLerp(10.0f, 140.0f, std::clamp((memory->globalVars->realtime - smoke.explosionTime) / animationDuration, 0.0f, 1.0f));
            if (ImVec2 screenPos; GameData::worldToScreen(smoke.origin + point * Vector{ radius, radius, radius * 0.7f }, screenPos)) {
                drawPrecomputedPrimitive(drawList, screenPos, color, vertices, indices);
            }
        }
    }
}

static void drawNadeBlast(ImDrawList* drawList) noexcept
{
    if (!miscConfig.nadeBlast.enabled)
        return;

    const auto color = Helpers::calculateColor(miscConfig.nadeBlast);

    static const auto spherePoints = generateSpherePoints<1000>();
    static const auto [vertices, indices] = generateAntialiasedDot();

    constexpr auto blastDuration = 0.35f;

    GameData::Lock lock;
    for (const auto& projectile : GameData::projectiles()) {
        if (!projectile.exploded || projectile.explosionTime + blastDuration < memory->globalVars->realtime)
            continue;

        for (const auto& point : spherePoints) {
            const auto radius = ImLerp(10.0f, 70.0f, (memory->globalVars->realtime - projectile.explosionTime) / blastDuration);
            if (ImVec2 screenPos; GameData::worldToScreen(projectile.coordinateFrame.origin() + point * radius, screenPos)) {
                drawPrecomputedPrimitive(drawList, screenPos, color, vertices, indices);
            }
        }
    }
}

static void hitEffect(ImDrawList* drawList, GameEvent* event = nullptr) noexcept
{
    if (!miscConfig.hitEffect.enabled)
        return;

    constexpr auto effectDuration = 0.3f;
    static float lastHitTime = 0.0f;

    if (event) {
        if (localPlayer && event->getInt("attacker") == localPlayer->getUserId())
            lastHitTime = memory->globalVars->realtime;
    } else if (lastHitTime + effectDuration >= memory->globalVars->realtime) {
        PostProcessing::performFullscreenChromaticAberration(drawList, (1.0f - (memory->globalVars->realtime - lastHitTime) / effectDuration) * 0.01f);
    }
}

void Misc::hitEffect(GameEvent& event) noexcept
{
    hitEffect(nullptr, &event);
}
    	
void Misc::panicKey() noexcept
{
    if (miscConfig.panicKey == -1)
        return;

	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(miscConfig.panicKey)))
        hooks->uninstall();
}

void Misc::hitMarker(GameEvent* event) noexcept
{
    if (!miscConfig.hitMarker.hitMarker.enabled || !localPlayer)
        return;

    static float lastHitTime = 0.0f;

    if (event && interfaces->engine->getPlayerForUserId(event->getInt("attacker")) == localPlayer->index()) {
        lastHitTime = memory->globalVars->realtime;
        return;
    }

    if (lastHitTime + miscConfig.hitMarker.hitMarkerTime < memory->globalVars->realtime)
        return;

    const auto ds = ImGui::GetIO().DisplaySize;

    const auto start = miscConfig.hitMarker.hitMarkerStart;
    const auto end = miscConfig.hitMarker.hitMarkerEnd;
    const auto width_mid = ds.x / 2;
    const auto height_mid = ds.y / 2;

    auto drawList = ImGui::GetBackgroundDrawList();
    const ImU32 color = Helpers::calculateColor(static_cast<Color>(miscConfig.hitMarker.hitMarker));
    drawList->AddLine({ width_mid + end, height_mid + end }, { width_mid + start, height_mid + start }, color, miscConfig.hitMarker.hitMarker.thickness);
    drawList->AddLine({ width_mid - end, height_mid + end }, { width_mid - start, height_mid + start }, color, miscConfig.hitMarker.hitMarker.thickness);
    drawList->AddLine({ width_mid + end, height_mid - end }, { width_mid + start, height_mid - start }, color, miscConfig.hitMarker.hitMarker.thickness);
    drawList->AddLine({ width_mid - end, height_mid - end }, { width_mid - start, height_mid - start }, color, miscConfig.hitMarker.hitMarker.thickness);
}

void Misc::drawPreESP(ImDrawList* drawList) noexcept
{
    drawMolotovHull(drawList);
    drawSmokeHull(drawList);
    drawNadeBlast(drawList);
}

void Misc::drawPostESP(ImDrawList* drawList) noexcept
{
    drawReloadProgress(drawList);
    drawRecoilCrosshair(drawList);
    purchaseList();
    drawObserverList();
    drawNoscopeCrosshair(drawList);
    drawFpsCounter();
    drawOffscreenEnemies(drawList);
    drawPlayerList();
    drawBombTimer();
    hitEffect(drawList);
    hitMarker();
}

void Misc::updateEventListeners(bool forceRemove) noexcept
{
    class PurchaseEventListener : public GameEventListener {
    public:
        void fireGameEvent(GameEvent* event) { purchaseList(event); }
    };

    static PurchaseEventListener listener;
    static bool listenerRegistered = false;

    if (miscConfig.purchaseList.enabled && !listenerRegistered) {
        interfaces->gameEventManager->addListener(&listener, "item_purchase");
        listenerRegistered = true;
    } else if ((!miscConfig.purchaseList.enabled || forceRemove) && listenerRegistered) {
        interfaces->gameEventManager->removeListener(&listener);
        listenerRegistered = false;
    }
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
    to_json(j, static_cast<const Color&>(o), static_cast<const Color&>(dummy));

    WRITE("Enabled", enabled)
    WRITE_OBJ("Health Bar", healthBar);
    WRITE("Audible Only", audibleOnly)
    WRITE("Spotted Only", spottedOnly)
}

static void to_json(json& j, const HitEffect& o, const HitEffect& dummy = {})
{
    WRITE("Enabled", enabled)
}

static void to_json(json& j, const PlayerList& o, const PlayerList& dummy = {})
{
    WRITE("Enabled", enabled)
    WRITE("Avatar", avatar)
    WRITE("Steam ID", steamID)
    WRITE("Rank", rank)
    WRITE("Wins", wins)
    WRITE("Money", money)
    WRITE("Health", health)
    WRITE("Armor", armor)
    WRITE("Last Place", lastPlace)

    if (const auto window = ImGui::FindWindowByName("Player List")) {
        j["Pos"] = window->Pos;
        j["Size"] = window->SizeFull;
    }
}

static void to_json(json& j, const HitMarker& o, const HitMarker& dummy = {})
{
    WRITE_OBJ("Hit Marker", hitMarker);
    WRITE("Hit Marker Start", hitMarkerStart);
    WRITE("Hit Marker End", hitMarkerEnd);
    WRITE("Hit Marker Time", hitMarkerTime);
}

json Misc::toJSON() noexcept
{
    json j;

    const auto& o = miscConfig;
    const decltype(miscConfig) dummy;

    WRITE_OBJ("Reload Progress", reloadProgress);
    WRITE("Ignore Flashbang", ignoreFlashbang);
    WRITE_OBJ("Recoil Crosshair", recoilCrosshair);
    WRITE_OBJ("Noscope Crosshair", noscopeCrosshair);
    WRITE_OBJ("Purchase List", purchaseList);
    WRITE_OBJ("Observer List", observerList);
    WRITE_OBJ("FPS Counter", fpsCounter);
    WRITE_OBJ("Offscreen Enemies", offscreenEnemies);
    WRITE_OBJ("Player List", playerList);
    WRITE_OBJ("Molotov Hull", molotovHull);
    WRITE_OBJ("Bomb Timer", bombTimer);
    WRITE_OBJ("Smoke Hull", smokeHull);
    WRITE_OBJ("Nade Blast", nadeBlast);
    WRITE_OBJ("Hit Effect", hitEffect);

    WRITE("Panic Key", panicKey);
    WRITE_OBJ("Hit Marker", hitMarker);
	
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
    from_json(j, static_cast<Color&>(o));

    read(j, "Enabled", o.enabled);
    read<value_t::object>(j, "Health Bar", o.healthBar);
    read(j, "Audible Only", o.audibleOnly);
    read(j, "Spotted Only", o.spottedOnly);
}

static void from_json(const json& j, HitEffect& o)
{
    read(j, "Enabled", o.enabled);
}

static void from_json(const json& j, PlayerList& o)
{
    read(j, "Enabled", o.enabled);
    read(j, "Avatar", o.avatar);
    read(j, "Steam ID", o.steamID);
    read(j, "Rank", o.rank);
    read(j, "Wins", o.wins);
    read(j, "Money", o.money);
    read(j, "Health", o.health);
    read(j, "Armor", o.armor);
    read(j, "Last Place", o.lastPlace);
    read<value_t::object>(j, "Pos", o.pos);
    read<value_t::object>(j, "Size", o.size);
}

static void from_json(const json& j, HitMarker& o)
{
    read<value_t::object>(j, "Hit Marker", o.hitMarker);
    read_number(j, "Hit Marker Start", o.hitMarkerStart);
    read_number(j, "Hit Marker End", o.hitMarkerEnd);
    read_number(j, "Hit Marker Time", o.hitMarkerTime);
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
    read<value_t::object>(j, "Player List", miscConfig.playerList);
    read<value_t::object>(j, "Molotov Hull", miscConfig.molotovHull);
    read<value_t::object>(j, "Bomb Timer", miscConfig.bombTimer);
    read<value_t::object>(j, "Smoke Hull", miscConfig.smokeHull);
    read<value_t::object>(j, "Nade Blast", miscConfig.nadeBlast);
    read<value_t::object>(j, "Hit Effect", miscConfig.hitEffect);

    read_number(j, "Panic Key", miscConfig.panicKey);
    read<value_t::object>(j, "Hit Marker", miscConfig.hitMarker);
}
