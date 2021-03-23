#include <numbers>
#include <numeric>
#include <ranges>
#include <sstream>
#include <unordered_map>
#include <vector>

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
#include "../SDK/NetworkChannel.h"
#include "../SDK/WeaponInfo.h"
#include "../SDK/WeaponSystem.h"

#include "../ImGuiCustom.h"
#include "../imgui/imgui.h"

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
    bool audibleOnly = false;
    bool spottedOnly = false;
};

struct PlayerList {
    bool enabled = false;
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

struct RainbowBar {
    ColorToggleThickness rainbowBar{ 3.0f };
    bool rainbowUp{ true };
    bool rainbowBottom{ false };
    bool rainbowLeft{ false };
    bool rainbowRight{ false };
    float rainbowScale{ 0.125f };
    bool rainbowPulse{ false };
    float rainbowPulseSpeed{ 1.0f };
};

struct StyleCustomEasy {
    Color BackGroundColor;
    Color TextColor;
    Color MainColor;
    Color MainAccentColor;
    Color HighlightColor;
};

struct Watermark {
    ColorToggle watermark;
    bool watermarkNickname{ true };
    bool watermarkUsername{ true };
    bool watermarkFPS{ true };
    bool watermarkPing{ true };
    bool watermarkTickrate{ true };
    bool watermarkVelocity{ true };
    bool watermarkTime{ true };
    bool watermarkAlpha{ true };
    ImVec2 watermarkPos{ 0.f,0.f };
    float watermarkScale{ 1.0f };
};

struct Plots
{
    bool enabled{ false };
    int FPS{ 0 };
    float FPSRefresh{ 60.f };
    int FPSInfo{ 2 };
    bool FPSStyle{ false };
    Color FPSStyleLines{};
    Color FPSStyleLinesHovered{};
    Color FPSStyleHistogram{};
    Color FPSStyleHistogramHovered{};
    ImVec2 FPSPos{ 0.f,0.f };
    ImVec2 FPSSize{ 200.f,80.f };
    float FPSScale{ 1.0f };
    int ping{ 0 };
    float pingRefresh{ 60.f };
    int pingInfo{ 2 };
    bool pingStyle{ false };
    Color pingStyleLines{};
    Color pingStyleLinesHovered{};
    Color pingStyleHistogram{};
    Color pingStyleHistogramHovered{};
    ImVec2 pingPos{ 0.f,0.f };
    ImVec2 pingSize{ 200.f,80.f };
    float pingScale{ 1.0f };
    int velocity{ 0 };
    float velocityRefresh{ 60.f };
    int velocityInfo{ 2 };
    bool velocityStyle{ false };
    Color velocityStyleLines{};
    Color velocityStyleLinesHovered{};
    Color velocityStyleHistogram{};
    Color velocityStyleHistogramHovered{};
    ImVec2 velocityPos{ 0.f,0.f };
    ImVec2 velocitySize{ 200.f,80.f };
    float velocityScale{ 1.0f };
};

struct HitMarker {
    ColorToggleThickness hitMarker;
    float hitMarkerLength{ 10.f };
    float hitMarkerTime{ 0.6f };
    ColorToggle hitMarkerDamageIndicator;
    bool hitMarkerDamageIndicatorCustomize{ false };
    int hitMarkerDamageIndicatorDist{ 50 };
    float hitMarkerDamageIndicatorRatio{ 0.6f };
};

#ifdef _WIN32
struct Radio
{
    int station{ 0 };
    float volume{ 10.f };
    bool mute{ false };
    std::array<std::string, 10> custom;
};
#endif

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

    RainbowBar rainbowBar;

    int menuColors{ 0 };
    StyleCustomEasy customEasy;
    Watermark watermark;
    Plots plots;
    HitMarker hitMarker;

#ifdef _WIN32
    Radio radio;
#endif
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

                    if (const auto it = std::ranges::find(GameData::players(), userId, &PlayerData::userId); it != GameData::players().cend()) {
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

    if (std::ranges::none_of(observers, [](const auto& obs) { return obs.targetIsLocalPlayer; }) && !gui->isOpen())
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

        if (const auto it = std::ranges::find(GameData::players(), observer.playerUserId, &PlayerData::userId); it != GameData::players().cend()) {
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

        if (player.fadingEndTime != 0.0f)
            Helpers::setAlphaFactor(player.fadingAlpha());
        const auto color = Helpers::calculateColor(255, 255, 255, 255);
        const auto triangleColor = Helpers::calculateColor(miscConfig.offscreenEnemies);
        Helpers::setAlphaFactor(1.0f);

        constexpr auto avatarRadius = 13.0f;
        constexpr auto triangleSize = 10.0f;

        const auto pos = ImGui::GetIO().DisplaySize / 2 + ImVec2{ x, y } * 200;
        const auto trianglePos = pos + ImVec2{ x, y } * (avatarRadius + 3);

        const ImVec2 trianglePoints[]{
            trianglePos + ImVec2{  0.4f * y, -0.4f * x } * triangleSize,
            trianglePos + ImVec2{  1.0f * x,  1.0f * y } * triangleSize,
            trianglePos + ImVec2{ -0.4f * y,  0.4f * x } * triangleSize
        };
        drawList->AddConvexPolyFilled(trianglePoints, 3, triangleColor);

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

static void updateColors() noexcept
{
    ImGuiStyle& style = ImGui::GetStyle();
    switch (miscConfig.menuColors) {
    case 0: ImGui::StyleColorsDark(); break;
    case 1: ImGui::StyleColorsLight(); break;
    case 2: ImGui::StyleColorsClassic(); break;
#pragma region Fatality
    case 5:
        style.Colors[ImGuiCol_Border] = ImVec4(0.227451f, 0.20784315f, 0.33333334f, 1.f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.1213873f, 0.12138609f, 0.12138609f, 0.f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.8745099f, 0.019607844f, 0.35686275f, 0.82080925f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.7490196f, 0.07058824f, 0.34509805f, 1.f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.8150289f, 0.014133479f, 0.31967682f, 1.f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.72156864f, 0.03137255f, 0.3254902f, 1.f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.f, 0.f, 0.f, 0.f);
        style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.7490196f, 0.07058824f, 0.34509805f, 0.90196085f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.10196079f, 0.07450981f, 0.24705884f, 1.f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.69364166f, 0.f, 0.26462612f, 1.f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.882353f, 0.054901965f, 0.37254903f, 0.40000004f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.68235296f, 0.039215688f, 0.3254902f, 0.30980393f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.92549026f, 0.03529412f, 0.3137255f, 0.90751445f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.9132948f, 0.f, 0.4065274f, 1.f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.f);
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.f, 0.f, 0.6242771f, 0.35f);
        style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.7490196f, 0.07058824f, 0.34509805f, 1.f);
        style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.87283236f, 0.1009055f, 0.42217016f, 1.f);
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.99421966f, 0.f, 0.37929693f, 0.7f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.7490196f, 0.07058824f, 0.34509805f, 1.f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.7490196f, 0.07058824f, 0.34509805f, 1.f);
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.7490196f, 0.07058824f, 0.34509805f, 1.f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.098039225f, 0.07058824f, 0.24313727f, 0.94117653f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.7668148f, 0.8135167f, 0.867052f, 0.f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.7490196f, 0.07058824f, 0.34509805f, 0.9490197f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.7490196f, 0.07058824f, 0.34509805f, 0.67058825f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.098039225f, 0.07058824f, 0.20000002f, 0.5294118f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.7411765f, 0.03529412f, 0.3254902f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.7490196f, 0.07058824f, 0.34509805f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.8265896f, 0.f, 0.3306363f, 1.f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.16338669f, 0.16338676f, 0.17341042f, 0.5f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(1.f, 0.f, 0.38690472f, 1.f);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.9537572f, 0.049617477f, 0.6140512f, 0.78f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.7490196f, 0.07058824f, 0.34509805f, 1.f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.68235296f, 0.039215688f, 0.3254902f, 1.f);
        style.Colors[ImGuiCol_Tab] = ImVec4(0.1440547f, 0.0012028452f, 0.20809251f, 0.f);
        style.Colors[ImGuiCol_TabActive] = ImVec4(0.9215687f, 0.039215688f, 0.30588236f, 0.8786127f);
        style.Colors[ImGuiCol_TabHovered] = ImVec4(0.68235296f, 0.039215688f, 0.3254902f, 0.9058824f);
        style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.12138609f, 0.12138662f, 0.1213873f, 0.9724f);
        style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.7490196f, 0.07058824f, 0.34509805f, 1.f);
        style.Colors[ImGuiCol_Text] = ImVec4(0.69411767f, 0.6666667f, 0.8313726f, 1.f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(1e-06f, 9.9999e-07f, 9.9999e-07f, 1.f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.7490196f, 0.07058824f, 0.34509805f, 0.34901962f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.08627451f, 0.07450981f, 0.18039216f, 1.f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.12941177f, 0.10196079f, 0.26666668f, 1.f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.63583815f, 0.040429037f, 0.4534296f, 0.51f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.12941177f, 0.10196079f, 0.26666668f, 1.f);
        break;
#pragma endregion
#pragma region OneTap-V3
    case 6:
        style.Colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.5f, 0.5f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.f, 0.f, 0.f, 0.f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.10980393f, 0.121568635f, 0.14901961f, 0.56647396f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.6308263f, 0.6570108f, 0.6820809f, 1.f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.93715084f, 0.6004459f, 0.19763936f, 1.f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.93725497f, 0.6f, 0.19607845f, 1.f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.43352604f, 0.4335217f, 0.4335217f, 0.f);
        style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.93725497f, 0.6f, 0.19607845f, 0.90196085f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.10980393f, 0.121568635f, 0.14901961f, 0.5411765f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.31139007f, 0.32974204f, 0.35055864f, 0.67f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.8670433f, 0.86704755f, 0.867052f, 0.4f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.80924046f, 0.80924416f, 0.80924857f, 0.31f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.93725497f, 0.6f, 0.19607845f, 1.f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.93725497f, 0.6f, 0.19607845f, 0.5803922f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14450872f, 0.14450727f, 0.14450727f, 1.f);
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.35f);
        style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.2318412f, 0.2318423f, 0.23184353f, 1.f);
        style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.28770947f, 0.28770658f, 0.28770658f, 0.7f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.93725497f, 0.6f, 0.19607845f, 1.f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.93725497f, 0.6f, 0.19607845f, 1.f);
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.32262564f, 0.3226228f, 0.32262242f, 1.f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.07122898f, 0.06970647f, 0.06724971f, 0.f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.93725497f, 0.6f, 0.19607845f, 0.9490197f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.83196896f, 0.848049f, 0.867052f, 0.67f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.93725497f, 0.6f, 0.19607845f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.5f, 0.5f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.6286879f, 0.68001086f, 0.7398844f, 1.f);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.5964116f, 0.6012691f, 0.60693645f, 0.78f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.93725497f, 0.6f, 0.19607845f, 1.f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.93725497f, 0.6f, 0.19607845f, 1.f);
        style.Colors[ImGuiCol_Tab] = ImVec4(0.050820325f, 0.051339746f, 0.052023172f, 0.f);
        style.Colors[ImGuiCol_TabActive] = ImVec4(0.1254902f, 0.13725491f, 0.1764706f, 1.f);
        style.Colors[ImGuiCol_TabHovered] = ImVec4(0.50288516f, 0.50288755f, 0.50289017f, 0.8f);
        style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.068f, 0.10199998f, 0.14800003f, 0.9724f);
        style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.8670433f, 0.8670472f, 0.867052f, 1.f);
        style.Colors[ImGuiCol_Text] = ImVec4(0.8588236f, 0.8705883f, 0.9058824f, 1.f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.5f, 0.5f, 1.f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.9075054f, 0.90750957f, 0.90751445f, 0.35f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.93725497f, 0.6f, 0.19607845f, 1.f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.93725497f, 0.6f, 0.19607845f, 1.f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.f, 0.f, 0.f, 0.51f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.16862746f, 0.18039216f, 0.21568629f, 1.f);
        break;
#pragma endregion
#pragma region Mutiny (WIP)
    case 7:
        style.Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.f);

        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.f);
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.f, 0.98f, 0.95f, 0.73f);



        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.f, 0.f, 1.f);
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.f, 0.f, 1.f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.f, 0.f, 0.f, 0.f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.56f, 0.56f, 0.58f, 1.f);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.f);





        style.Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.f, 0.f, 0.43f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.f, 0.98f, 0.95f, 0.75f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.f);
        break;
#pragma endregion
#pragma region CSGOSimple (WIP)
    case 8:
        style.FrameRounding = 0.f;
        style.WindowRounding = 0.f;
        style.ChildRounding = 0.f;

        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.f, 0.009f, 0.120f, 0.940f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.076f, 0.143f, 0.209f, 1.f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.30f, 1.f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.260f, 0.590f, 0.980f, 0.670f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.260f, 0.590f, 0.980f, 0.670f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.260f, 0.590f, 0.980f, 1.f);    break;
#pragma endregion
#pragma region CSGOSimple(Extender) (WIP)
    case 9:
        style.WindowPadding = ImVec2(13, 13);
        style.WindowRounding = 0.f;
        style.FramePadding = ImVec2(5, 5);
        style.FrameRounding = 0.f;
        style.ItemSpacing = ImVec2(12, 8);
        style.ItemInnerSpacing = ImVec2(8, 6);
        style.IndentSpacing = 25.0f;
        style.ScrollbarSize = 15.0f;
        style.ScrollbarRounding = 9.0f;
        style.GrabMinSize = 5.0f;
        style.GrabRounding = 3.0f;

        style.Colors[ImGuiCol_Text] = ImVec4(0.83f, 0.95f, 0.95f, 1.f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.39f, 0.80f, 0.80f, 1.f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.200f, 0.200f, 0.200f, 0.847f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.f, 0.f, 0.f, 0.f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.90f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.502f, 0.502f, 0.502f, 1.f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.f, 0.f, 0.f, 0.f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.55f, 0.55f, 0.67f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.75f, 0.75f, 0.67f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.49f, 0.49f, 0.45f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.16f, 0.49f, 0.49f, 1.f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.56f, 0.56f, 1.f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.49f, 0.49f, 0.60f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.16f, 0.47f, 0.47f, 1.f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.f, 0.f, 0.f, 0.f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.f, 0.f, 0.f, 0.f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.f, 0.f, 0.f, 0.f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.f, 0.f, 0.f, 0.f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.44f, 0.81f, 0.81f, 1.f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.33f, 0.78f, 0.78f, 0.60f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.33f, 0.78f, 0.78f, 1.f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.902f, 0.596f, 0.302f, 0.929f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.902f, 0.596f, 0.302f, 1.f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.f, 0.596f, 0.302f, 1.f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.15f, 0.44f, 0.44f, 1.f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.33f, 0.78f, 0.78f, 0.67f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.33f, 0.78f, 0.78f, 1.f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.25f, 0.74f, 0.74f, 0.40f);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.25f, 0.74f, 0.74f, 0.60f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.25f, 0.74f, 0.74f, 1.f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.20f, 0.61f, 0.61f, 1.f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.53f, 0.84f, 0.84f, 0.67f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.53f, 0.84f, 0.84f, 1.f);





        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.53f, 0.84f, 0.84f, 1.f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.f, 0.84f, 0.84f, 1.f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.53f, 0.84f, 0.84f, 1.f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.f, 0.84f, 0.84f, 1.f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.13f, 0.40f, 0.40f, 1.f);




        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.09f, 0.27f, 0.27f, 0.67f);    break;
#pragma endregion
#pragma region BoyNextHook (WIP)
    case 10:
        style.Colors[ImGuiCol_Text] = ImVec4(1.f, 1.f, 1.f, 1.f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(33 / 255.f, 35 / 255.f, 47 / 255.f, 1.f);
        style.Colors[ImGuiCol_ChildBg] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        style.Colors[ImGuiCol_Border] = ImVec4(30 / 255.f, 30 / 255.f, 41 / 255.f, 1.f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.f, 0.f, 0.f, 0.f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(33 / 255.f, 33 / 255.f, 33 / 255.f, 1.f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.15f, 0.15f, 1.f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.08f, 0.08f, 0.08f, 1.f);
        style.Colors[ImGuiCol_TitleBg] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(35 / 255.f, 35 / 255.f, 35 / 255.f, 1.f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_MenuBarBg] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ScrollbarBg] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ScrollbarGrab] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = style.Colors[ImGuiCol_ScrollbarGrabHovered];
        style.Colors[ImGuiCol_CheckMark] = ImVec4(167 / 255.f, 24 / 255.f, 71 / 255.f, 1.f);


        style.Colors[ImGuiCol_Button] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ButtonHovered] = style.Colors[ImGuiCol_TitleBgActive]; //
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(135 / 255.f, 135 / 255.f, 135 / 255.f, 1.f); //
        style.Colors[ImGuiCol_Header] = style.Colors[ImGuiCol_CheckMark]; //multicombo, combo selected item color.
        style.Colors[ImGuiCol_HeaderHovered] = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_HeaderActive] = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Text];
        style.Colors[ImGuiCol_SeparatorHovered] = style.Colors[ImGuiCol_Text];
        style.Colors[ImGuiCol_SeparatorActive] = style.Colors[ImGuiCol_Text];
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);





        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.f, 0.43f, 0.35f, 1.f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.f, 1.f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.f, 0.60f, 0.f, 1.f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        style.Colors[ImGuiCol_DragDropTarget] = style.Colors[ImGuiCol_FrameBgActive];



        break;
#pragma endregion
#pragma region NIXWARE (WIP)
    case 11:
        style.Alpha = 1.0f; // Global alpha applies to everything in ImGui
        style.WindowPadding = ImVec2(8, 8);      // Padding within a window
        style.WindowRounding = 0.0f; // Radius of window corners rounding. Set to 0.0f to have rectangular windows
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);// Alignment for title bar text
        style.FramePadding = ImVec2(4, 1);      // Padding within a framed rectangle (used by most widgets)
        style.FrameRounding = 0.0f; // Radius of frame corners rounding. Set to 0.0f to have rectangular frames (used by most widgets).
        style.ItemSpacing = ImVec2(8, 4);      // Horizontal and vertical spacing between widgets/lines
        style.ItemInnerSpacing = ImVec2(4, 4);      // Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label)
        style.TouchExtraPadding = ImVec2(0, 0);      // Expand reactive bounding box for touch-based system where touch position is not accurate enough. Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much!
        style.IndentSpacing = 21.0f;// Horizontal spacing when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2).
        style.ColumnsMinSpacing = 6.0f; // Minimum horizontal spacing between two columns
        style.ScrollbarSize = 10.0f;// Width of the vertical scrollbar, Height of the horizontal scrollbar
        style.ScrollbarRounding = 3.0f; // Radius of grab corners rounding for scrollbar
        style.GrabMinSize = 10.0f;// Minimum width/height of a grab box for slider/scrollbar
        style.GrabRounding = 0.0f; // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
        style.ButtonTextAlign = ImVec2(0.5f, 0.5f);// Alignment of button text when button is larger than text.
        style.DisplayWindowPadding = ImVec2(22, 22);    // Window positions are clamped to be visible within the display area by at least this amount. Only covers regular windows.
        style.DisplaySafeAreaPadding = ImVec2(4, 4);      // If you cannot see the edge of your screen (e.g. on a TV) increase the safe area padding. Covers popups/tooltips as well regular windows.
        style.AntiAliasedLines = true; // Enable anti-aliasing on lines/borders. Disable if you are really short on CPU/GPU.
        style.CurveTessellationTol = 1.25f;// Tessellation tolerance. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality.

        style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);





        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);




        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);    break;
#pragma endregion
#pragma region Setupim (WIP)
    case 12:
        style.Alpha = 1.f;
        style.FrameRounding = 3.0f;

        style.Colors[ImGuiCol_Text] = ImVec4(0.f, 0.f, 0.f, 1.f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.f, 0.f, 0.f, 0.f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(1.f, 1.f, 1.f, 0.94f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.f, 0.f, 0.f, 0.39f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.f, 1.f, 1.f, 0.10f);
        style.Colors[ImGuiCol_FrameBg] = style.Colors[ImGuiCol_PopupBg];
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.f, 1.f, 1.f, 0.51f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.f);
        style.Colors[ImGuiCol_SliderGrabActive] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_Button] = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_ButtonHovered] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        style.Colors[ImGuiCol_HeaderActive] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_Separator] = ImVec4(0.39f, 0.39f, 0.39f, 1.f);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
        style.Colors[ImGuiCol_SeparatorActive] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.f, 1.f, 1.f, 0.50f);
        style.Colors[ImGuiCol_ResizeGripHovered] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);





        style.Colors[ImGuiCol_PlotLines] = style.Colors[ImGuiCol_Separator];
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.f, 0.43f, 0.35f, 1.f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.f, 1.f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.f, 0.60f, 0.f, 1.f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);




        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);    break;
#pragma endregion
#pragma region Monochrome (WIP)
    case 13:
        style.WindowRounding = 5.3f;
        style.FrameRounding = 2.3f;
        style.ScrollbarRounding = 0;

        style.Colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 0.90f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.15f, 1.f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.f, 0.f, 0.f, 0.f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.85f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.70f, 0.70f, 0.70f, 0.65f);
        style.Colors[ImGuiCol_BorderShadow] = style.Colors[ImGuiCol_ChildBg];
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.f, 0.f, 0.01f, 1.f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.80f, 0.80f, 0.40f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.90f, 0.65f, 0.65f, 0.45f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.f, 0.f, 0.f, 0.83f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.f, 0.f, 0.f, 0.87f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.01f, 0.01f, 0.02f, 0.80f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.55f, 0.53f, 0.55f, 0.51f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.56f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.91f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.83f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.70f, 0.70f, 0.70f, 0.62f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.30f, 0.30f, 0.30f, 0.84f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.48f, 0.72f, 0.89f, 0.49f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.50f, 0.69f, 0.99f, 0.68f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.80f, 0.50f, 0.50f, 1.f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.30f, 0.69f, 1.f, 0.53f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.44f, 0.61f, 0.86f, 1.f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.38f, 0.62f, 0.83f, 1.f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.50f, 0.50f, 1.f);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.70f, 0.60f, 0.60f, 1.f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.90f, 0.70f, 0.70f, 1.f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.f, 1.f, 1.f, 0.85f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.f, 1.f, 1.f, 0.60f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.f, 1.f, 1.f, 0.90f);





        style.Colors[ImGuiCol_PlotLines] = ImVec4(1.f, 1.f, 1.f, 1.f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.f, 1.f);
        style.Colors[ImGuiCol_PlotHistogram] = style.Colors[ImGuiCol_PlotLinesHovered];
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.f, 0.60f, 0.f, 1.f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.f, 0.f, 1.f, 0.35f);




        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);    break;
#pragma endregion
#pragma region Procedural (WIP)
    case 14:
        style.Colors[ImGuiCol_Text] = ImVec4(236.f / 255.f, 240.f / 255.f, 241.f / 255.f, 1.f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(236.f / 255.f, 240.f / 255.f, 241.f / 255.f, 0.58f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(44.f / 255.f, 62.f / 255.f, 80.f / 255.f, 0.95f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(57.f / 255.f, 79.f / 255.f, 105.f / 255.f, 0.58f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(33.f / 255.f, 46.f / 255.f, 60.f / 255.f, 0.92f);
        style.Colors[ImGuiCol_Border] = ImVec4(44.f / 255.f, 62.f / 255.f, 80.f / 255.f, 0.f);
        style.Colors[ImGuiCol_BorderShadow] = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_FrameBg] = ImVec4(57.f / 255.f, 79.f / 255.f, 105.f / 255.f, 1.f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 0.78f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 1.f);
        style.Colors[ImGuiCol_TitleBg] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_TitleBgActive] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(57.f / 255.f, 79.f / 255.f, 105.f / 255.f, 0.75f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(57.f / 255.f, 79.f / 255.f, 105.f / 255.f, 0.47f);
        style.Colors[ImGuiCol_ScrollbarBg] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 0.21f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_ScrollbarGrabActive] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_CheckMark] = ImVec4(41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 0.80f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 0.50f);
        style.Colors[ImGuiCol_SliderGrabActive] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Button] = style.Colors[ImGuiCol_SliderGrab];
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 0.86f);
        style.Colors[ImGuiCol_ButtonActive] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Header] = ImVec4(41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 0.76f);
        style.Colors[ImGuiCol_HeaderHovered] = style.Colors[ImGuiCol_ButtonHovered];
        style.Colors[ImGuiCol_HeaderActive] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Separator] = ImVec4(41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 0.32f);
        style.Colors[ImGuiCol_SeparatorHovered] = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_SeparatorActive] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 0.15f);
        style.Colors[ImGuiCol_ResizeGripHovered] = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_ResizeGripActive] = style.Colors[ImGuiCol_FrameBgActive];





        style.Colors[ImGuiCol_PlotLines] = ImVec4(236.f / 255.f, 240.f / 255.f, 241.f / 255.f, 0.63f);
        style.Colors[ImGuiCol_PlotLinesHovered] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_PlotHistogram] = style.Colors[ImGuiCol_PlotLines];
        style.Colors[ImGuiCol_PlotHistogramHovered] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 0.43f);




        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(57.f / 255.f, 79.f / 255.f, 105.f / 255.f, 0.73f);    break;
#pragma endregion
#pragma region Pagghiu (WIP)
    case 15:
        style.Alpha = 1.f;
        style.FrameRounding = 4;
        style.IndentSpacing = 12.0f;

        style.Colors[ImGuiCol_Text] = ImVec4(0.31f, 0.25f, 0.24f, 1.f);

        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.68f, 0.68f, 0.68f, 0.f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.82f, 0.92f, 1.f, 0.90f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.f, 0.f, 0.f, 0.f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.62f, 0.70f, 0.72f, 0.56f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.95f, 0.33f, 0.14f, 0.47f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.97f, 0.31f, 0.13f, 0.81f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.42f, 0.75f, 1.f, 0.53f);

        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.65f, 0.80f, 0.20f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.74f, 0.74f, 0.94f, 1.f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.40f, 0.62f, 0.80f, 0.15f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.39f, 0.64f, 0.80f, 0.30f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.28f, 0.67f, 0.80f, 0.59f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.25f, 0.48f, 0.53f, 0.67f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.48f, 0.47f, 0.47f, 0.71f);

        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.31f, 0.47f, 0.99f, 1.f);
        style.Colors[ImGuiCol_Button] = ImVec4(1.f, 0.79f, 0.18f, 0.78f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.42f, 0.82f, 1.f, 0.81f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.72f, 1.f, 1.f, 0.86f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.65f, 0.78f, 0.84f, 0.80f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.75f, 0.88f, 0.94f, 0.80f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.55f, 0.68f, 0.74f, 0.80f);



        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.60f, 0.60f, 0.80f, 0.30f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.f, 1.f, 1.f, 0.60f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.f, 1.f, 1.f, 0.90f);









        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.f, 0.99f, 0.54f, 0.43f);




        break;
#pragma endregion
#pragma region Doug (WIP)
    case 16:
        style.Colors[ImGuiCol_Text] = ImVec4(1.f, 1.f, 1.f, 1.f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(1.f, 1.f, 1.f, 0.f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.f, 0.f, 0.f, 0.94f);
        style.Colors[ImGuiCol_Border] = ImVec4(1.f, 1.f, 1.f, 0.19f);

        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.f, 0.f, 0.f, 0.51f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.f);
        style.Colors[ImGuiCol_SliderGrabActive] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_Button] = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_ButtonHovered] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        style.Colors[ImGuiCol_HeaderActive] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
        style.Colors[ImGuiCol_SeparatorActive] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
        style.Colors[ImGuiCol_ResizeGripHovered] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);





        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.f, 0.43f, 0.35f, 1.f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.f, 1.f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.f, 0.60f, 0.f, 1.f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);

        style.Colors[ImGuiCol_NavHighlight] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.f, 1.f, 1.f, 0.12f);

        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);    break;
#pragma endregion
#pragma region Microsoft (WIP)
    case 17:
        style.DisplaySafeAreaPadding = ImVec2(0, 0);
        style.WindowPadding = ImVec2(4, 6);
        style.FramePadding = ImVec2(8, 6);
        style.ItemSpacing = style.FramePadding;
        style.ItemInnerSpacing = style.FramePadding;
        style.IndentSpacing = 20.f;

        style.WindowRounding = 0.f;
        style.FrameRounding = style.WindowRounding;

        style.WindowBorderSize = style.WindowRounding;
        style.FrameBorderSize = 1.f;
        style.PopupBorderSize = style.FrameBorderSize;

        style.ScrollbarSize = 20.f;
        style.ScrollbarRounding = style.WindowRounding;

        style.GrabMinSize = 5.0f;
        style.GrabRounding = style.WindowRounding;

        style.Colors[ImGuiCol_Text] = ImVec4(0.10f, 0.10f, 0.10f, 1.f);

        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.95f, 0.95f, 0.95f, 1.f);
        style.Colors[ImGuiCol_ChildBg] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_PopupBg] = ImVec4(1.f, 1.f, 1.f, 1.f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.60f, 0.60f, 0.60f, 1.f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.f, 0.f, 0.f, 0.f);
        style.Colors[ImGuiCol_FrameBg] = style.Colors[ImGuiCol_PopupBg];
        style.Colors[ImGuiCol_FrameBgHovered] = style.Colors[ImGuiCol_ButtonHovered];
        style.Colors[ImGuiCol_FrameBgActive] = style.Colors[ImGuiCol_ButtonActive];



        style.Colors[ImGuiCol_MenuBarBg] = style.Colors[ImGuiCol_Button];
        style.Colors[ImGuiCol_ScrollbarBg] = style.Colors[ImGuiCol_Button];
        style.Colors[ImGuiCol_ScrollbarGrab] = style.Colors[ImGuiCol_SliderGrab];
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.f, 0.f, 0.f, 0.20f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = style.Colors[ImGuiCol_SliderGrabActive];
        style.Colors[ImGuiCol_CheckMark] = style.Colors[ImGuiCol_Text];
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.f, 0.f, 0.f, 0.50f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.86f, 0.86f, 0.86f, 1.f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.f, 0.47f, 0.84f, 0.20f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.f, 0.47f, 0.84f, 1.f);
        style.Colors[ImGuiCol_Header] = style.Colors[ImGuiCol_Button];
        style.Colors[ImGuiCol_HeaderHovered] = style.Colors[ImGuiCol_ButtonHovered];
        style.Colors[ImGuiCol_HeaderActive] = style.Colors[ImGuiCol_ButtonActive];




















        break;
#pragma endregion
#pragma region Darcula (WIP)
    case 18:
        style.WindowRounding = 5.3f;
        style.GrabRounding = style.FrameRounding = 2.3f;
        style.FrameBorderSize = 1.f;
        style.ScrollbarRounding = 5.0f;
        style.ItemSpacing.y = 6.5f;

        style.Colors[ImGuiCol_Text] = ImVec4(0.73333335f, 0.73333335f, 0.73333335f, 1.f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.34509805f, 0.34509805f, 0.34509805f, 1.f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.23529413f, 0.24705884f, 0.25490198f, 0.94f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.23529413f, 0.24705884f, 0.25490198f, 0.f);
        style.Colors[ImGuiCol_PopupBg] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_Border] = ImVec4(0.33333334f, 0.33333334f, 0.33333334f, 0.50f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.15686275f, 0.15686275f, 0.15686275f, 0.f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.16862746f, 0.16862746f, 0.16862746f, 0.54f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.453125f, 0.67578125f, 0.99609375f, 0.67f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.47058827f, 0.47058827f, 0.47058827f, 0.67f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.29f, 0.48f, 1.f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.f, 0.f, 0.f, 0.51f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.27058825f, 0.28627452f, 0.2901961f, 0.80f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.27058825f, 0.28627452f, 0.2901961f, 0.60f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.21960786f, 0.30980393f, 0.41960788f, 0.51f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.21960786f, 0.30980393f, 0.41960788f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.13725491f, 0.19215688f, 0.2627451f, 0.91f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.83f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.70f, 0.70f, 0.70f, 0.62f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.30f, 0.30f, 0.30f, 0.84f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.33333334f, 0.3529412f, 0.36078432f, 0.49f);
        style.Colors[ImGuiCol_ButtonHovered] = style.Colors[ImGuiCol_ScrollbarGrabHovered];
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.13725491f, 0.19215688f, 0.2627451f, 1.f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.33333334f, 0.3529412f, 0.36078432f, 0.53f);
        style.Colors[ImGuiCol_HeaderHovered] = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_HeaderActive] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Separator] = ImVec4(0.31640625f, 0.31640625f, 0.31640625f, 1.f);
        style.Colors[ImGuiCol_SeparatorHovered] = style.Colors[ImGuiCol_Separator];
        style.Colors[ImGuiCol_SeparatorActive] = style.Colors[ImGuiCol_Separator];
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.f, 1.f, 1.f, 0.85f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.f, 1.f, 1.f, 0.60f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.f, 1.f, 1.f, 0.90f);





        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.f, 0.43f, 0.35f, 1.f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.f, 1.f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.f, 0.60f, 0.f, 1.f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.18431373f, 0.39607847f, 0.79215693f, 0.90f);




        break;
#pragma endregion
#pragma region UE4 (Unreal Engine 4) (WIP)
    case 19:
        style.Colors[ImGuiCol_Text] = ImVec4(1.f, 1.f, 1.f, 1.f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(1.f, 1.f, 1.f, 0.f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.f, 0.f, 0.f, 0.f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.21f, 0.22f, 0.54f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.40f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 0.67f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.29f, 0.29f, 0.29f, 1.f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.f, 0.f, 0.f, 0.51f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.94f, 0.94f, 0.94f, 1.f);
        style.Colors[ImGuiCol_SliderGrab] = style.Colors[ImGuiCol_ScrollbarGrabActive];
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.86f, 0.86f, 0.86f, 1.f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.46f, 0.47f, 0.48f, 1.f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.70f, 0.70f, 0.70f, 0.31f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.70f, 0.70f, 0.70f, 0.80f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.48f, 0.50f, 0.52f, 1.f);
        style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.72f, 0.72f, 0.72f, 0.78f);
        style.Colors[ImGuiCol_SeparatorActive] = style.Colors[ImGuiCol_ScrollbarGrabActive];
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);





        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.f, 0.43f, 0.35f, 1.f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.73f, 0.60f, 0.15f, 1.f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.f, 0.60f, 0.f, 1.f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.87f, 0.87f, 0.87f, 0.35f);
        style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.f, 1.f, 0.f, 0.90f);
        style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.f);
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.f, 1.f, 1.f, 0.70f);

        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);    break;
#pragma endregion
#pragma region Cherry (WIP)
    case 20:
        style.WindowPadding = ImVec2(6, 4);
        style.WindowRounding = 0.f;
        style.WindowBorderSize = 1.f;
        style.WindowTitleAlign.x = 0.50f;
        style.FramePadding = ImVec2(5, 2);
        style.FrameRounding = 3.0f;
        style.FrameBorderSize = 0.f;
        style.ItemSpacing = ImVec2(7, 1);
        style.ItemInnerSpacing = ImVec2(1, 1);
        style.TouchExtraPadding = ImVec2(0, 0);
        style.IndentSpacing = 6.0f;
        style.ScrollbarSize = 12.0f;
        style.ScrollbarRounding = 16.0f;
        style.GrabMinSize = 20.f;
        style.GrabRounding = 2.0f;

        style.Colors[ImGuiCol_Text] = ImVec4(0.860f, 0.930f, 0.890f, 0.78f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.860f, 0.930f, 0.890f, 0.28f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.17f, 1.f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.200f, 0.220f, 0.270f, 0.58f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.200f, 0.220f, 0.270f, 0.9f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.539f, 0.479f, 0.255f, 0.162f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.f, 0.f, 0.f, 0.f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.200f, 0.220f, 0.270f, 1.f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.455f, 0.198f, 0.301f, 0.78f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.455f, 0.198f, 0.301f, 1.f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.232f, 0.201f, 0.271f, 1.f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.502f, 0.075f, 0.256f, 1.f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.200f, 0.220f, 0.270f, 0.75f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.200f, 0.220f, 0.270f, 0.47f);
        style.Colors[ImGuiCol_ScrollbarBg] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.09f, 0.15f, 0.16f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_ScrollbarGrabActive] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.71f, 0.22f, 0.27f, 1.f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
        style.Colors[ImGuiCol_SliderGrabActive] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_Button] = style.Colors[ImGuiCol_SliderGrab];
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.455f, 0.198f, 0.301f, 0.86f);
        style.Colors[ImGuiCol_ButtonActive] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Header] = ImVec4(0.455f, 0.198f, 0.301f, 0.76f);
        style.Colors[ImGuiCol_HeaderHovered] = style.Colors[ImGuiCol_ButtonHovered];
        style.Colors[ImGuiCol_HeaderActive] = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_Separator] = ImVec4(0.14f, 0.16f, 0.19f, 1.f);
        style.Colors[ImGuiCol_SeparatorHovered] = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_SeparatorActive] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.47f, 0.77f, 0.83f, 0.04f);
        style.Colors[ImGuiCol_ResizeGripHovered] = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_ResizeGripActive] = style.Colors[ImGuiCol_FrameBgActive];





        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.860f, 0.930f, 0.890f, 0.63f);
        style.Colors[ImGuiCol_PlotLinesHovered] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_PlotHistogram] = style.Colors[ImGuiCol_PlotLines];
        style.Colors[ImGuiCol_PlotHistogramHovered] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.455f, 0.198f, 0.301f, 0.43f);




        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.200f, 0.220f, 0.270f, 0.73f);    break;
#pragma endregion
#pragma region LightGreen (WIP)
    case 21:
        style.WindowRounding = 2.0f; // Radius of window corners rounding. Set to 0.0f to have rectangular windows

        style.ScrollbarRounding = 3.0f; // Radius of grab corners rounding for scrollbar
        style.ScrollbarSize = 16;

        style.FrameRounding = style.ScrollbarRounding;
        style.FramePadding.x = 6;
        style.FramePadding.y = 4;

        style.GrabRounding = style.WindowRounding; // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
        style.ChildRounding = style.WindowRounding;
        style.IndentSpacing = 22;
        style.ItemSpacing.x = 10;
        style.ItemSpacing.y = 4;
        style.AntiAliasedLines = true;
        style.AntiAliasedFill = true;
        style.Alpha = 1.0f;

        style.Colors[ImGuiCol_Text] = ImVec4(0.f, 0.f, 0.f, 1.f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.f, 0.f, 0.f, 0.f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.93f, 0.93f, 0.93f, 0.98f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.71f, 0.71f, 0.71f, 0.08f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.f, 0.f, 0.f, 0.04f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.71f, 0.71f, 0.71f, 0.55f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.94f, 0.94f, 0.94f, 0.55f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.71f, 0.78f, 0.69f, 0.98f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.85f, 0.85f, 0.85f, 1.f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.78f, 0.78f, 0.78f, 1.f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.82f, 0.78f, 0.78f, 0.51f);
        style.Colors[ImGuiCol_MenuBarBg] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.61f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.90f, 0.90f, 0.90f, 0.30f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.92f, 0.92f, 0.92f, 0.78f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.f, 1.f, 1.f, 1.f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.184f, 0.407f, 0.193f, 1.f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.71f, 0.78f, 0.69f, 0.40f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.725f, 0.805f, 0.702f, 1.f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.793f, 0.900f, 0.836f, 1.f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.71f, 0.78f, 0.69f, 0.31f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.71f, 0.78f, 0.69f, 0.80f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.71f, 0.78f, 0.69f, 1.f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.39f, 0.39f, 0.39f, 1.f);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.14f, 0.44f, 0.80f, 1.f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.f, 1.f, 1.f, 0.f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.45f);
        style.Colors[ImGuiCol_ResizeGripActive] = style.Colors[ImGuiCol_SliderGrab];





        style.Colors[ImGuiCol_PlotLines] = style.Colors[ImGuiCol_Separator];
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.f, 0.43f, 0.35f, 1.f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.f, 1.f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.f, 0.60f, 0.f, 1.f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        style.Colors[ImGuiCol_NavHighlight] = style.Colors[ImGuiCol_HeaderHovered];
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);

        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);    break;
#pragma endregion
#pragma region Photoshop
    case 22:
        style.FrameBorderSize = 1.f;
        style.GrabMinSize = 7.0f;
        style.ScrollbarSize = 13.0f;
        style.TabBorderSize = style.FrameBorderSize;

        style.ChildRounding = 4.0f;
        style.FrameRounding = 2.0f;
        style.ScrollbarRounding = 12.0f;
        style.TabRounding = 0.f;
        style.PopupRounding = style.FrameRounding;
        style.WindowRounding = style.ChildRounding;

        style.Colors[ImGuiCol_Text] = ImVec4(1.f, 1.f, 1.f, 1.f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.500f, 0.500f, 0.500f, 1.f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.180f, 0.180f, 0.180f, 1.f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.280f, 0.280f, 0.280f, 0.f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.313f, 0.313f, 0.313f, 1.f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.266f, 0.266f, 0.266f, 1.f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.f, 0.f, 0.f, 0.f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.200f, 0.200f, 0.200f, 1.f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.280f, 0.280f, 0.280f, 1.f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.148f, 0.148f, 0.148f, 1.f);
        style.Colors[ImGuiCol_TitleBgActive] = style.Colors[ImGuiCol_TitleBg];
        style.Colors[ImGuiCol_TitleBgCollapsed] = style.Colors[ImGuiCol_TitleBg];
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.195f, 0.195f, 0.195f, 1.f);
        style.Colors[ImGuiCol_ScrollbarBg] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.277f, 0.277f, 0.277f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.300f, 0.300f, 0.300f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.f, 0.391f, 0.f, 1.f);
        style.Colors[ImGuiCol_CheckMark] = style.Colors[ImGuiCol_Text];
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.391f, 0.391f, 0.391f, 1.f);
        style.Colors[ImGuiCol_SliderGrabActive] = style.Colors[ImGuiCol_ScrollbarGrabActive];
        style.Colors[ImGuiCol_Button] = ImVec4(1.f, 1.f, 1.f, 0.f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.f, 1.f, 1.f, 0.156f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.f, 1.f, 1.f, 0.391f);
        style.Colors[ImGuiCol_Header] = style.Colors[ImGuiCol_PopupBg];
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.469f, 0.469f, 0.469f, 1.f);
        style.Colors[ImGuiCol_HeaderActive] = style.Colors[ImGuiCol_HeaderHovered];
        style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_SeparatorHovered] = style.Colors[ImGuiCol_SliderGrab];
        style.Colors[ImGuiCol_SeparatorActive] = style.Colors[ImGuiCol_ScrollbarGrabActive];
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.f, 1.f, 1.f, 0.250f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.f, 1.f, 1.f, 0.670f);
        style.Colors[ImGuiCol_ResizeGripActive] = style.Colors[ImGuiCol_ScrollbarGrabActive];
        style.Colors[ImGuiCol_Tab] = ImVec4(0.098f, 0.098f, 0.098f, 1.f);
        style.Colors[ImGuiCol_TabHovered] = ImVec4(0.352f, 0.352f, 0.352f, 1.f);
        style.Colors[ImGuiCol_TabActive] = style.Colors[ImGuiCol_MenuBarBg];
        style.Colors[ImGuiCol_TabUnfocused] = style.Colors[ImGuiCol_Tab];
        style.Colors[ImGuiCol_TabUnfocusedActive] = style.Colors[ImGuiCol_MenuBarBg];
        style.Colors[ImGuiCol_PlotLines] = style.Colors[ImGuiCol_HeaderHovered];
        style.Colors[ImGuiCol_PlotLinesHovered] = style.Colors[ImGuiCol_ScrollbarGrabActive];
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.586f, 0.586f, 0.586f, 1.f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = style.Colors[ImGuiCol_ScrollbarGrabActive];
        style.Colors[ImGuiCol_TextSelectedBg] = style.Colors[ImGuiCol_ButtonHovered];
        style.Colors[ImGuiCol_DragDropTarget] = style.Colors[ImGuiCol_ScrollbarGrabActive];
        style.Colors[ImGuiCol_NavHighlight] = style.Colors[ImGuiCol_ScrollbarGrabActive];
        style.Colors[ImGuiCol_NavWindowingHighlight] = style.Colors[ImGuiCol_ScrollbarGrabActive];
        style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.f, 0.f, 0.f, 0.586f);
        style.Colors[ImGuiCol_ModalWindowDimBg] = style.Colors[ImGuiCol_NavWindowingDimBg];    break;
#pragma endregion
#pragma region CorporateGrey
    case 23:
        style.ItemSpacing = ImVec2(6, 2);

        style.WindowRounding = 3;
        style.WindowPadding = ImVec2(4, 4);
        style.WindowBorderSize = 1;

        style.ChildRounding = style.WindowRounding;
        style.ChildBorderSize = style.WindowBorderSize;

        style.FrameRounding = style.WindowRounding;
        style.FramePadding = style.ItemSpacing;
        style.FrameBorderSize = 0;

        style.ScrollbarRounding = 2;
        style.ScrollbarSize = 18;

        style.PopupRounding = style.WindowRounding;
        style.PopupBorderSize = style.WindowBorderSize;

        style.GrabRounding = style.WindowRounding;

        style.TabBorderSize = style.FrameBorderSize;
        style.TabRounding = style.WindowRounding;

        style.Colors[ImGuiCol_Text] = ImVec4(1.f, 1.f, 1.f, 1.f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.f);
        style.Colors[ImGuiCol_ChildBg] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_PopupBg] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_Border] = ImVec4(0.12f, 0.12f, 0.12f, 0.71f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.f, 1.f, 1.f, 0.06f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.42f, 0.42f, 0.42f, 0.54f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.42f, 0.42f, 0.42f, 0.40f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.67f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.19f, 0.19f, 0.19f, 1.f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.17f, 0.17f, 0.17f, 0.90f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.335f, 0.335f, 0.335f, 1.f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.24f, 0.24f, 0.24f, 0.53f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.41f, 0.41f, 0.41f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.52f, 0.52f, 0.52f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.65f, 0.65f, 0.65f, 1.f);
        style.Colors[ImGuiCol_SliderGrab] = style.Colors[ImGuiCol_ScrollbarGrabHovered];
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.64f, 0.64f, 0.64f, 1.f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.54f, 0.54f, 0.54f, 0.35f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.52f, 0.52f, 0.52f, 0.59f);
        style.Colors[ImGuiCol_ButtonActive] = style.Colors[ImGuiCol_ScrollbarGrabActive];
        style.Colors[ImGuiCol_Header] = ImVec4(0.38f, 0.38f, 0.38f, 1.f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.47f, 0.47f, 0.47f, 1.f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.76f, 0.76f, 0.76f, 0.77f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.f, 0.f, 0.f, 0.137f);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.700f, 0.671f, 0.600f, 0.290f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.702f, 0.671f, 0.600f, 0.674f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        style.Colors[ImGuiCol_Tab] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        style.Colors[ImGuiCol_TabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        style.Colors[ImGuiCol_TabActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
        style.Colors[ImGuiCol_TabUnfocused] = style.Colors[ImGuiCol_Tab];
        style.Colors[ImGuiCol_TabUnfocusedActive] = style.Colors[ImGuiCol_TabActive];
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.f, 0.43f, 0.35f, 1.f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.f, 1.f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.f, 0.60f, 0.f, 1.f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.73f, 0.73f, 0.73f, 0.35f);
        style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.f, 1.f, 0.f, 0.90f);
        style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.f);
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.f, 1.f, 1.f, 0.70f);
        style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);    break;
#pragma endregion
#pragma region Violet
    case 24:
        style.Colors[ImGuiCol_Text] = ImVec4(0.956863f, 0.945098f, 0.870588f, 0.8f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.356863f, 0.345098f, 0.270588f, 0.8f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.145098f, 0.129412f, 0.192157f, 0.8f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.f, 0.f, 0.f, 0.2f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.145098f, 0.129412f, 0.192157f, 0.901961f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.545098f, 0.529412f, 0.592157f, 0.8f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.f, 0.f, 0.f, 0.8f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.47451f, 0.137255f, 0.34902f, 0.4f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.67451f, 0.337255f, 0.54902f, 0.4f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.57451f, 0.237255f, 0.44902f, 1.f);
        style.Colors[ImGuiCol_TitleBg] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.245098f, 0.229412f, 0.292157f, 1.f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = style.Colors[ImGuiCol_BorderShadow];
        style.Colors[ImGuiCol_MenuBarBg] = style.Colors[ImGuiCol_BorderShadow];
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.545098f, 0.529412f, 0.592157f, 0.501961f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.445098f, 0.429412f, 0.492157f, 0.8f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.645098f, 0.629412f, 0.692157f, 0.8f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.545098f, 0.529412f, 0.592157f, 1.f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.780392f, 0.937255f, 0.f, 0.8f);
        style.Colors[ImGuiCol_SliderGrab] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.880392f, 1.03725f, 0.1f, 1.f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.854902f, 0.0666667f, 0.368627f, 0.8f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0549f, 0.266667f, 0.568627f, 0.8f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.954902f, 0.166667f, 0.468627f, 1.f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.47451f, 0.137255f, 0.34902f, 0.8f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.67451f, 0.337255f, 0.54902f, 0.8f);
        style.Colors[ImGuiCol_HeaderActive] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.745098f, 0.729412f, 0.792157f, 0.8f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.645098f, 0.629412f, 0.692157f, 1.f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.854902f, 0.0666667f, 0.368627f, 0.2f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0549f, 0.266667f, 0.568627f, 0.2f);
        style.Colors[ImGuiCol_ResizeGripActive] = style.Colors[ImGuiCol_ButtonActive];
        style.Colors[ImGuiCol_Tab] = ImVec4(0.854902f, 0.0666667f, 0.368627f, 0.6f);
        style.Colors[ImGuiCol_TabHovered] = ImVec4(1.0549f, 0.266667f, 0.568627f, 0.6f);
        style.Colors[ImGuiCol_TabActive] = style.Colors[ImGuiCol_ButtonActive];
        style.Colors[ImGuiCol_TabUnfocused] = style.Colors[ImGuiCol_Tab];
        style.Colors[ImGuiCol_TabUnfocusedActive] = style.Colors[ImGuiCol_TabActive];
        style.Colors[ImGuiCol_PlotLines] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.980392f, 1.13725f, 0.2f, 0.8f);
        style.Colors[ImGuiCol_PlotHistogram] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_PlotHistogramHovered] = style.Colors[ImGuiCol_PlotLinesHovered];
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.780392f, 0.937255f, 0.f, 0.4f);
        style.Colors[ImGuiCol_DragDropTarget] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_NavHighlight] = ImVec4(1.f, 1.f, 1.f, 0.8f);
        style.Colors[ImGuiCol_NavWindowingHighlight] = style.Colors[ImGuiCol_NavHighlight];
        style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.f, 1.f, 1.f, 0.2f);
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.f, 0.f, 0.f, 0.6f);

        style.WindowMenuButtonPosition = ImGuiDir_Right;    break;
#pragma endregion
#pragma region Raikiri
    case 25:
        style.FrameRounding = 4.0f;
        style.GrabRounding = style.FrameRounding;

        style.Colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.f, 0.f, 0.f, 0.f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
        style.Colors[ImGuiCol_TitleBgActive] = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.f, 0.f, 0.f, 0.51f);
        style.Colors[ImGuiCol_MenuBarBg] = style.Colors[ImGuiCol_ChildBg];
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
        style.Colors[ImGuiCol_ScrollbarGrab] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.21f, 0.31f, 1.f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.f, 1.f);
        style.Colors[ImGuiCol_SliderGrab] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.f, 1.f);
        style.Colors[ImGuiCol_Button] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ButtonHovered] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.f);
        style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        style.Colors[ImGuiCol_Tab] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_TabHovered] = style.Colors[ImGuiCol_HeaderHovered];
        style.Colors[ImGuiCol_TabActive] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_TabUnfocused] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_TabUnfocusedActive] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.f, 0.43f, 0.35f, 1.f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.f, 1.f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.f, 0.60f, 0.f, 1.f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.f, 1.f, 0.f, 0.90f);
        style.Colors[ImGuiCol_NavHighlight] = style.Colors[ImGuiCol_HeaderActive];
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.f, 1.f, 1.f, 0.70f);
        style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);    break;
#pragma endregion
#pragma region VGUI (Valve GUI)
    case 26:
        style.FrameRounding = 0.f;
        style.FrameBorderSize = 1.f;

        style.WindowRounding = style.FrameRounding;
        style.ChildRounding = style.FrameRounding;
        style.PopupRounding = style.FrameRounding;
        style.ScrollbarRounding = style.FrameRounding;
        style.GrabRounding = style.FrameRounding;
        style.TabRounding = style.FrameRounding;

        style.Colors[ImGuiCol_Text] = ImVec4(1.f, 1.f, 1.f, 1.f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.29f, 0.34f, 0.26f, 1.f);
        style.Colors[ImGuiCol_ChildBg] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.24f, 0.27f, 0.20f, 1.f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.54f, 0.57f, 0.51f, 0.50f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.14f, 0.16f, 0.11f, 0.52f);
        style.Colors[ImGuiCol_FrameBg] = style.Colors[ImGuiCol_PopupBg];
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.27f, 0.30f, 0.23f, 1.f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.34f, 0.26f, 1.f);
        style.Colors[ImGuiCol_TitleBg] = style.Colors[ImGuiCol_PopupBg];
        style.Colors[ImGuiCol_TitleBgActive] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.f, 0.f, 0.f, 0.51f);
        style.Colors[ImGuiCol_MenuBarBg] = style.Colors[ImGuiCol_PopupBg];
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.35f, 0.42f, 0.31f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.28f, 0.32f, 0.24f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.25f, 0.30f, 0.22f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.23f, 0.27f, 0.21f, 1.f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.59f, 0.54f, 0.18f, 1.f);
        style.Colors[ImGuiCol_SliderGrab] = style.Colors[ImGuiCol_ScrollbarBg];
        style.Colors[ImGuiCol_SliderGrabActive] = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_Button] = ImVec4(0.29f, 0.34f, 0.26f, 0.40f);
        style.Colors[ImGuiCol_ButtonHovered] = style.Colors[ImGuiCol_ScrollbarBg];
        style.Colors[ImGuiCol_ButtonActive] = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_Header] = style.Colors[ImGuiCol_ScrollbarBg];
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.35f, 0.42f, 0.31f, 0.6f);
        style.Colors[ImGuiCol_HeaderActive] = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_Separator] = ImVec4(0.14f, 0.16f, 0.11f, 1.f);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.54f, 0.57f, 0.51f, 1.f);
        style.Colors[ImGuiCol_SeparatorActive] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.19f, 0.23f, 0.18f, 0.f); // grip invis
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.54f, 0.57f, 0.51f, 1.f);
        style.Colors[ImGuiCol_ResizeGripActive] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_Tab] = style.Colors[ImGuiCol_ScrollbarBg];
        style.Colors[ImGuiCol_TabHovered] = ImVec4(0.54f, 0.57f, 0.51f, 0.78f);
        style.Colors[ImGuiCol_TabActive] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_TabUnfocused] = style.Colors[ImGuiCol_PopupBg];
        style.Colors[ImGuiCol_TabUnfocusedActive] = style.Colors[ImGuiCol_ScrollbarBg];
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.f);
        style.Colors[ImGuiCol_PlotLinesHovered] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.f, 0.78f, 0.28f, 1.f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.f, 0.60f, 0.f, 1.f);
        style.Colors[ImGuiCol_TextSelectedBg] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.73f, 0.67f, 0.24f, 1.f);
        style.Colors[ImGuiCol_NavHighlight] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.f, 1.f, 1.f, 0.70f);
        style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);    break;
#pragma endregion
#pragma region Gold & Black
    case 27:
        style.ItemSpacing = ImVec2(10, 2);
        style.IndentSpacing = 12;

        style.WindowRounding = 4;
        style.WindowTitleAlign = ImVec2(1.f, 0.5f);
        style.WindowMenuButtonPosition = ImGuiDir_Right;

        style.ScrollbarRounding = 6;
        style.ScrollbarSize = 10;

        style.FrameRounding = style.WindowRounding;
        style.FramePadding = ImVec2(4, 2);

        style.PopupRounding = style.WindowRounding;
        style.GrabRounding = style.WindowRounding;
        style.TabRounding = style.WindowRounding;

        style.DisplaySafeAreaPadding = ImVec2(4, 4);

        style.Colors[ImGuiCol_Text] = ImVec4(0.92f, 0.92f, 0.92f, 1.f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.44f, 0.44f, 0.44f, 1.f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.f, 0.f, 0.f, 0.f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.51f, 0.36f, 0.15f, 1.f);
        style.Colors[ImGuiCol_BorderShadow] = style.Colors[ImGuiCol_ChildBg];
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.f);
        style.Colors[ImGuiCol_FrameBgHovered] = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.f);
        style.Colors[ImGuiCol_TitleBg] = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.91f, 0.64f, 0.13f, 1.f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.f, 0.f, 0.f, 0.51f);
        style.Colors[ImGuiCol_MenuBarBg] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.53f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.21f, 0.21f, 0.21f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.47f, 0.47f, 0.47f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.81f, 0.83f, 0.81f, 1.f);
        style.Colors[ImGuiCol_CheckMark] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_SliderGrab] = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_SliderGrabActive] = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_Button] = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_ButtonHovered] = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_ButtonActive] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Header] = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_HeaderHovered] = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.93f, 0.65f, 0.14f, 1.f);
        style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_ScrollbarGrab];
        style.Colors[ImGuiCol_SeparatorHovered] = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_SeparatorActive] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_ResizeGrip] = style.Colors[ImGuiCol_ScrollbarGrab];
        style.Colors[ImGuiCol_ResizeGripHovered] = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_ResizeGripActive] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Tab] = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_TabHovered] = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_TabActive] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
        style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.f);
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.f, 0.43f, 0.35f, 1.f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.f, 1.f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.f, 0.60f, 0.f, 1.f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.f, 1.f, 0.f, 0.90f);
        style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.f);
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.f, 1.f, 1.f, 0.70f);
        style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);    break;
#pragma endregion
#pragma region Sewer
    case 28:
        style.FrameRounding = 4.0f;
        style.GrabRounding = style.FrameRounding;

        style.WindowBorderSize = 0.f;
        style.PopupBorderSize = style.WindowBorderSize;

        style.Colors[ImGuiCol_Text] = ImVec4(1.f, 1.f, 1.f, 1.f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.73f, 0.75f, 0.74f, 1.f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.09f, 0.94f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.f, 0.f, 0.f, 0.f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.20f, 0.50f);
        style.Colors[ImGuiCol_BorderShadow] = style.Colors[ImGuiCol_ChildBg];
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.71f, 0.39f, 0.39f, 0.54f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.84f, 0.66f, 0.66f, 0.40f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.84f, 0.66f, 0.66f, 0.67f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.47f, 0.22f, 0.22f, 0.67f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.47f, 0.22f, 0.22f, 1.f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = style.Colors[ImGuiCol_TitleBg];
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.34f, 0.16f, 0.16f, 1.f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.f);
        style.Colors[ImGuiCol_CheckMark] = style.Colors[ImGuiCol_Text];
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.71f, 0.39f, 0.39f, 1.f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.84f, 0.66f, 0.66f, 1.f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.47f, 0.22f, 0.22f, 0.65f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.71f, 0.39f, 0.39f, 0.65f);
        style.Colors[ImGuiCol_ButtonActive] = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_Header] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.84f, 0.66f, 0.66f, 0.65f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.84f, 0.66f, 0.66f, 0.f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        style.Colors[ImGuiCol_SeparatorHovered] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_SeparatorActive] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ResizeGrip] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.84f, 0.66f, 0.66f, 0.66f);
        style.Colors[ImGuiCol_ResizeGripActive] = style.Colors[ImGuiCol_ResizeGripHovered];
        style.Colors[ImGuiCol_Tab] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_TabHovered] = style.Colors[ImGuiCol_ResizeGripHovered];
        style.Colors[ImGuiCol_TabActive] = style.Colors[ImGuiCol_ResizeGripHovered];
        style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
        style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.f);
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.f, 0.43f, 0.35f, 1.f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.f, 1.f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.f, 0.60f, 0.f, 1.f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.f, 1.f, 0.f, 0.90f);
        style.Colors[ImGuiCol_NavHighlight] = style.Colors[ImGuiCol_ScrollbarGrabHovered];
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.f, 1.f, 1.f, 0.70f);
        style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);    break;
#pragma endregion
#pragma region VS (Visual Studio) (WIP)
    case 29:
        style.WindowRounding = 0.f;
        style.ChildRounding = style.WindowRounding;
        style.FrameRounding = style.WindowRounding;
        style.GrabRounding = style.WindowRounding;
        style.PopupRounding = style.WindowRounding;
        style.ScrollbarRounding = style.WindowRounding;
        style.TabRounding = style.WindowRounding;

        style.Colors[ImGuiCol_Text] = ImVec4(1.f, 1.f, 1.f, 1.f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.59215686274f, 0.59215686274f, 0.59215686274f, 1.f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.14509803921f, 0.14509803921f, 0.14901960784f, 1.f);
        style.Colors[ImGuiCol_ChildBg] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_PopupBg] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_Border] = ImVec4(0.30588235294f, 0.30588235294f, 0.30588235294f, 1.f);
        style.Colors[ImGuiCol_BorderShadow] = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.21568627451f, 1.f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.11372549019f, 0.59215686274f, 0.92549019607f, 1.f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.f, 0.46666666666f, 0.78431372549f, 1.f);
        style.Colors[ImGuiCol_TitleBg] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_TitleBgActive] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_TitleBgCollapsed] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_MenuBarBg] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ScrollbarBg] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.32156862745f, 0.32156862745f, 0.33333333333f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35294117647f, 0.35294117647f, 0.3725490196f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = style.Colors[ImGuiCol_ScrollbarGrabHovered];
        style.Colors[ImGuiCol_CheckMark] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_SliderGrab] = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_SliderGrabActive] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Button] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ButtonHovered] = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_ButtonActive] = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_Header] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_HeaderHovered] = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_HeaderActive] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_SeparatorHovered] = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_SeparatorActive] = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_ResizeGrip] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_ResizeGripHovered] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ResizeGripActive] = style.Colors[ImGuiCol_ScrollbarGrab];
        style.Colors[ImGuiCol_Tab] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_TabHovered] = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_TabActive] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_TabUnfocused] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_TabUnfocusedActive] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_PlotLines] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_PlotLinesHovered] = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_PlotHistogram] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_PlotHistogramHovered] = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_TextSelectedBg] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_DragDropTarget] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_NavHighlight] = style.Colors[ImGuiCol_WindowBg];


        style.Colors[ImGuiCol_ModalWindowDimBg] = style.Colors[ImGuiCol_WindowBg];    break;
#pragma endregion
#pragma region OverShifted
    case 30:
        style.GrabRounding = 2.3f;
        style.FrameRounding = style.GrabRounding;

        style.Colors[ImGuiCol_Text] = ImVec4(1.f, 1.f, 1.f, 1.f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.f);
        style.Colors[ImGuiCol_ChildBg] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_PopupBg] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.f, 0.f, 0.f, 0.f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.f);
        style.Colors[ImGuiCol_TitleBgActive] = style.Colors[ImGuiCol_TitleBg];
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.f, 0.f, 0.f, 0.51f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.11f, 0.64f, 0.92f, 1.f);
        style.Colors[ImGuiCol_SliderGrab] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.08f, 0.50f, 0.72f, 1.f);
        style.Colors[ImGuiCol_Button] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ButtonHovered] = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_ButtonActive] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.22f, 1.f);
        style.Colors[ImGuiCol_HeaderHovered] = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_HeaderActive] = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.41f, 0.42f, 0.44f, 1.f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        style.Colors[ImGuiCol_ResizeGrip] = style.Colors[ImGuiCol_BorderShadow];
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.29f, 0.30f, 0.31f, 0.67f);
        style.Colors[ImGuiCol_ResizeGripActive] = style.Colors[ImGuiCol_SeparatorActive];
        style.Colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.09f, 0.83f);
        style.Colors[ImGuiCol_TabHovered] = ImVec4(0.33f, 0.34f, 0.36f, 0.83f);
        style.Colors[ImGuiCol_TabActive] = ImVec4(0.23f, 0.23f, 0.24f, 1.f);
        style.Colors[ImGuiCol_TabUnfocused] = style.Colors[ImGuiCol_TitleBg];
        style.Colors[ImGuiCol_TabUnfocusedActive] = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.f, 0.43f, 0.35f, 1.f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.f, 1.f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.f, 0.60f, 0.f, 1.f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        style.Colors[ImGuiCol_DragDropTarget] = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.f);
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.f, 1.f, 1.f, 0.70f);
        style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);    break;
#pragma endregion
#pragma region RayTeak
    case 31:
        style.WindowPadding = ImVec2(15, 15);
        style.WindowRounding = 5.0f;
        style.FramePadding = ImVec2(5, 5);
        style.FrameRounding = 4.0f;
        style.ItemSpacing = ImVec2(12, 8);
        style.ItemInnerSpacing = ImVec2(8, 6);
        style.IndentSpacing = 25.0f;
        style.ScrollbarSize = 15.0f;
        style.ScrollbarRounding = 9.0f;
        style.GrabMinSize = 5.0f;
        style.GrabRounding = 3.0f;

        style.Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);

        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);





        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);




        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);    break;
#pragma endregion
    }
}

#ifdef _WIN32
#include "../BASS/bass.h"
std::atomic<HSTREAM> radioStream(NULL);
std::vector<const char*> radioNames{ "Off",
            "RadioRecord",
            "RadioRecord: Супердискотека 90-х",
            "RadioRecord: Trancemission",
            "RadioRecord: Russian Mix",
            "RadioRecord: Медляк FM",
            "RadioRecord: Гоп FM",
            "RadioRecord: Vip Mix",
            "RadioRecord: Pirate Station",
            "RadioRecord: Yo! FM",
            "RadioRecord: Pump'n'Klubb",
            "RadioRecord: Teodor Hardstyle",
            "RadioRecord: Record Chill-Out",
            "RadioRecord: Record Club",
            "RadioRecord: Record Deep",
            "RadioRecord: Record Breaks",
            "RadioRecord: Record Dancecore",
            "RadioRecord: Record Dubstep",
            "RadioRecord: Record Trap",
            "RadioRecord: Record Techno",
            "RadioRecord: Minimal Techno",
            "RadioRecord: Future House",
            "RadioRecord: Rock Radio",
            "Nightwave Plaza",
            "WGFM Главный канал",
            "WGFM Второй канал",
            "WGFM Trance",
            "WGFM Rock",
            "Хіт FM",
            "Хіт FM Українські хіти",
            "Хіт FM Найбільші хіти",
            "Хіт FM Сучасні хіти",
            "NRJ",
            "NRJ Hot 40",
            "NRJ All Hits",
            "NRJ Party Hits",
            "KISS FM",
            "KISS FM Ukrainian",
            "KISS FM Deep",
            "KISS FM Black (+18)",
            "KISS FM Digital",
            "KISS FM Trendz",
            "Radio ROKS",
            "Radio ROKS: Український рок",
            "Radio ROKS: Новий Рок",
            "Radio ROKS: Hard'n'Heavy",
            "Radio ROKS: Рок-Балади",
            "Европа-Плюс",
            "Европа-Плюс: Light",
            "Европа-Плюс: Residance" };
std::string radioStations[] = {
    // RadioRecord: http://www.radiorecord.fm/
    "https://air.radiorecord.ru:8101/rr_320",						// Основной
    "https://air.radiorecord.ru:8102/sd90_320",						// Супердискотека 90-х
    "https://air.radiorecord.ru:8102/tm_320",						// Trancemission
    "https://air.radiorecord.ru:8102/rus_320",						// Russian Mix
    "https://air.radiorecord.ru:8102/mdl_320",						// Медляк FM
    "https://air.radiorecord.ru:8102/gop_320",						// Гоп FM
    "https://air.radiorecord.ru:8102/vip_320",						// Vip Mix
    "https://air.radiorecord.ru:8102/ps_320",						// Pirate Station
    "https://air.radiorecord.ru:8102/yo_320",						// Yo! FM
    "https://air.radiorecord.ru:8102/pump_320",						// Pump'n'Klubb
    "https://air.radiorecord.ru:8102/teo_320",						// Teodor Hardstyle
    "https://air.radiorecord.ru:8102/chil_320",						// Record Chill-Out
    "https://air.radiorecord.ru:8102/club_320",						// Record Club
    "https://air.radiorecord.ru:8102/deep_320",						// Record Deep
    "https://air.radiorecord.ru:8102/brks_320",						// Record Breaks
    "https://air.radiorecord.ru:8102/dc_320",						// Record Dancecore
    "https://air.radiorecord.ru:8102/dub_320",						// Record Dubstep
    "https://air.radiorecord.ru:8102/trap_320",						// Record Trap
    "https://air.radiorecord.ru:8102/techno_320",					// Record Techno
    "https://air.radiorecord.ru:8102/mini_320",						// Minimal Techno
    "https://air.radiorecord.ru:8102/fut_320",						// Future House
    "https://air.radiorecord.ru:8102/rock_320",						// Rock Radio

    "https://radio.plaza.one/mp3",									// Nightwave Plaza (https://plaza.one)

    // Wargaming.fm: http://wargaming.fm/
    "https://sv.wargaming.fm/1/128",								// WGFM Главный канал
    "https://sv.wargaming.fm/2/128",								// WGFM Второй канал
    "https://sv.wargaming.fm/3/128",								// WGFM Trance
    "https://sv.wargaming.fm/4/128",								// WGFM Rock

    // Xiт FM: https://www.hitfm.ua/
    "https://online.hitfm.ua/HitFM_HD",						        // Хіт FM
    "https://online.hitfm.ua/HitFM_Ukr_HD",					        // Хіт FM Українські хіти
    "https://online.hitfm.ua/HitFM_Best_HD",					    // Хіт FM Найбільші хіти
    "https://online.hitfm.ua/HitFM_Top_HD",					        // Хіт FM Сучасні хіти

    // NRJ Украина: http://nrj.ua/
    "https://cast.radiogroup.com.ua/nrj",							// NRJ
    "https://cast.radiogroup.com.ua/nrj_hot",						// NRJ Hot 40
    "https://cast.radiogroup.com.ua/nrj_hits",						// NRJ All Hits
    "https://cast.radiogroup.com.ua/nrj_party",						// NRJ Party Hits

    // Kiss FM: https://www.kissfm.ua/
    "https://online.kissfm.ua/KissFM_HD",						    // Ефір KISS FM
    "https://online.kissfm.ua/KissFM_Ukr_HD",				        // KISS FM Ukrainian
    "https://online.kissfm.ua/KissFM_Deep_HD",				        // KISS FM Deep
    "https://online.kissfm.ua/KissFM_Black_HD",			            // KISS FM Black (+18)
    "https://online.kissfm.ua/KissFM_Digital_HD",			        // KISS FM Digital
    "https://online.kissfm.ua/KissFM_Trendz_HD",			        // KISS FM Trendz

    // Radio ROKS: https://www.radioroks.ua/
    "https://online-radioroks2.tavrmedia.ua/RadioROKS",				// Ефір Radio ROKS
    "https://online-radioroks2.tavrmedia.ua/RadioROKS_Ukr",			// Український рок
    "https://online-radioroks2.tavrmedia.ua/RadioROKS_NewRock",		// Новий Рок
    "https://online-radioroks2.tavrmedia.ua/RadioROKS_HardnHeavy",	// Hard'n'Heavy
    "https://online-radioroks2.tavrmedia.ua/RadioROKS_Ballads",		// Рок-Балади

    // Европа-Плюс: http://www.europaplus.ru/
    "https://ep256.hostingradio.ru:8052/europaplus256.mp3",			// Основное
    "https://emg02.hostingradio.ru/ep-light128.mp3",				// Light
    "https://emg02.hostingradio.ru/ep-residance128.mp3",			// Residance
};

static void updateRadio(bool off = false) noexcept
{
    if (radioStream) {
        BASS_ChannelStop(radioStream);
        radioStream = NULL;
    }
    if (!off)
        std::thread([]() {
        radioStream = BASS_StreamCreateURL(radioStations[miscConfig.radio.station - 1].c_str(), 0, 0, NULL, 0);
            }).detach();
}

static void radio() noexcept
{
    static bool radioInit = false;
    if (!radioInit) {
        BASS_Init(-1, 44100, BASS_DEVICE_3D, 0, NULL);
        radioInit = true;
    }
    if (miscConfig.radio.station && radioStream)
    {
        BASS_ChannelSetAttribute(radioStream, BASS_ATTRIB_VOL, (miscConfig.radio.mute ? 0.f : miscConfig.radio.volume) / 100.0f);
        BASS_ChannelPlay(radioStream, false);
    }
}
#endif

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

    ImGui::PushID("Player List");
    ImGui::Checkbox("Player List", &miscConfig.playerList.enabled);
    ImGui::SameLine();

    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
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

    ImGuiCustom::colorPicker("Rainbow Bar", miscConfig.rainbowBar.rainbowBar);
    if (miscConfig.rainbowBar.rainbowBar.enabled) {
        ImGui::SameLine();
        ImGui::PushID("Rainbow Bar");
        if (ImGui::Button("..."))
            ImGui::OpenPopup("RB");

        if (ImGui::BeginPopup("RB")) {
            ImGui::Text("Position:");
            ImGui::Checkbox("Upper", &miscConfig.rainbowBar.rainbowUp);
            ImGui::Checkbox("Bottom", &miscConfig.rainbowBar.rainbowBottom);
            ImGui::Checkbox("Left", &miscConfig.rainbowBar.rainbowLeft);
            ImGui::Checkbox("Right", &miscConfig.rainbowBar.rainbowRight);
            ImGui::Text("Scale:");
            ImGui::SliderFloat("Scale", &miscConfig.rainbowBar.rainbowScale, 0.03125f, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic);
            ImGui::Text("Scale presets:");
            if (ImGui::Button("0.25x"))
                miscConfig.rainbowBar.rainbowScale = 0.03125f;
            ImGui::SameLine();
            if (ImGui::Button("0.5x"))
                miscConfig.rainbowBar.rainbowScale = 0.0625f;
            ImGui::SameLine();
            if (ImGui::Button("1x"))
                miscConfig.rainbowBar.rainbowScale = 0.125f;
            ImGui::SameLine();
            if (ImGui::Button("2x"))
                miscConfig.rainbowBar.rainbowScale = 0.25f;
            ImGui::SameLine();
            if (ImGui::Button("4x"))
                miscConfig.rainbowBar.rainbowScale = 0.5f;
            ImGui::SameLine();
            if (ImGui::Button("8x"))
                miscConfig.rainbowBar.rainbowScale = 1.0f;
            ImGui::Text("Pulse:");
            ImGui::Checkbox("Enable", &miscConfig.rainbowBar.rainbowPulse);
            ImGui::SliderFloat("Speed", &miscConfig.rainbowBar.rainbowPulseSpeed, 0.1f, 25.0f, "%.1f", ImGuiSliderFlags_Logarithmic);
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    ImGuiCustom::colorPicker("Watermark", miscConfig.watermark.watermark);
    if (miscConfig.watermark.watermark.enabled) {
        ImGui::SameLine();
        ImGui::PushID("Watermark");
        if (ImGui::Button("..."))
            ImGui::OpenPopup("WM");

        if (ImGui::BeginPopup("WM")) {
            ImGui::Checkbox("Nickname (Only in game)", &miscConfig.watermark.watermarkNickname);
            ImGui::Checkbox("Username", &miscConfig.watermark.watermarkUsername);
            ImGui::Checkbox("FPS", &miscConfig.watermark.watermarkFPS);
            ImGui::Checkbox("Ping", &miscConfig.watermark.watermarkPing);
            ImGui::Checkbox("Tickrate", &miscConfig.watermark.watermarkTickrate);
            ImGui::Checkbox("Velocity", &miscConfig.watermark.watermarkVelocity);
            ImGui::Checkbox("Time", &miscConfig.watermark.watermarkTime);
            ImGui::Checkbox("Alpha", &miscConfig.watermark.watermarkAlpha);
            ImGui::DragFloat("Scale", &miscConfig.watermark.watermarkScale, 0.005f, 0.3f, 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    ImGui::Checkbox("Plots", &miscConfig.plots.enabled);
    if (miscConfig.plots.enabled) {
        ImGui::SameLine();
        ImGui::PushID("Plots");
        if (ImGui::Button("..."))
            ImGui::OpenPopup("P");

        if (ImGui::BeginPopup("P")) {
            ImGui::Combo("FPS", &miscConfig.plots.FPS, "Off\0Lines\0Histogram\0");
            if (miscConfig.plots.FPS) {
                ImGui::Combo("FPS Info", &miscConfig.plots.FPSInfo, "Off\0Name\0Full\0");
                ImGui::Checkbox("FPS Custom Style", &miscConfig.plots.FPSStyle);
                if (miscConfig.plots.FPSStyle)
                {
                    if (miscConfig.plots.FPS == 1)
                    {
                        ImGuiCustom::colorPicker("FPS Lines Color", miscConfig.plots.FPSStyleLines);
                        ImGuiCustom::colorPicker("FPS Lines Hovered Color", miscConfig.plots.FPSStyleLinesHovered);
                    }
                    else if (miscConfig.plots.FPS == 2)
                    {
                        ImGuiCustom::colorPicker("FPS Histogram Color", miscConfig.plots.FPSStyleHistogram);
                        ImGuiCustom::colorPicker("FPS Histogram Hovered Color", miscConfig.plots.FPSStyleHistogramHovered);
                    }
                }
                ImGui::DragFloat("FPS Refresh rate", &miscConfig.plots.FPSRefresh, 1.f, 1.f, 200.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                ImGui::DragFloat("FPS Size X", &miscConfig.plots.FPSSize.x, 1.f, 20.f, 400.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                ImGui::DragFloat("FPS Size Y", &miscConfig.plots.FPSSize.y, 1.f, 10.f, 200.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            }
            ImGui::Combo("Ping", &miscConfig.plots.ping, "Off\0Lines\0Histogram\0");
            if (miscConfig.plots.ping) {
                ImGui::Combo("Ping Info", &miscConfig.plots.pingInfo, "Off\0Name\0Full\0");
                ImGui::Checkbox("Ping Custom Style", &miscConfig.plots.pingStyle);
                if (miscConfig.plots.pingStyle)
                {
                    if (miscConfig.plots.ping == 1)
                    {
                        ImGuiCustom::colorPicker("Ping Lines Color", miscConfig.plots.pingStyleLines);
                        ImGuiCustom::colorPicker("Ping Lines Hovered Color", miscConfig.plots.pingStyleLinesHovered);
                    }
                    else if (miscConfig.plots.ping == 2)
                    {
                        ImGuiCustom::colorPicker("Ping Histogram Color", miscConfig.plots.pingStyleHistogram);
                        ImGuiCustom::colorPicker("Ping Histogram Hovered Color", miscConfig.plots.pingStyleHistogramHovered);
                    }
                }
                ImGui::DragFloat("Ping Refresh rate", &miscConfig.plots.pingRefresh, 1.f, 1.f, 200.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                ImGui::DragFloat("Ping Size X", &miscConfig.plots.pingSize.x, 1.f, 20.f, 400.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                ImGui::DragFloat("Ping Size Y", &miscConfig.plots.pingSize.y, 1.f, 10.f, 200.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            }
            ImGui::Combo("Velocity", &miscConfig.plots.velocity, "Off\0Lines\0Histogram\0");
            if (miscConfig.plots.velocity) {
                ImGui::Combo("Velocity Info", &miscConfig.plots.velocityInfo, "Off\0Name\0Full\0");
                ImGui::Checkbox("Velocity Custom Style", &miscConfig.plots.velocityStyle);
                if (miscConfig.plots.velocityStyle)
                {
                    if (miscConfig.plots.velocity == 1)
                    {
                        ImGuiCustom::colorPicker("Velocity Lines Color", miscConfig.plots.velocityStyleLines);
                        ImGuiCustom::colorPicker("Velocity Lines Hovered Color", miscConfig.plots.velocityStyleLinesHovered);
                    }
                    else if (miscConfig.plots.velocity == 2)
                    {
                        ImGuiCustom::colorPicker("Velocity Histogram Color", miscConfig.plots.velocityStyleHistogram);
                        ImGuiCustom::colorPicker("Velocity Histogram Hovered Color", miscConfig.plots.velocityStyleHistogramHovered);
                    }
                }
                ImGui::DragFloat("Velocity Refresh rate", &miscConfig.plots.velocityRefresh, 1.f, 1.f, 200.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                ImGui::DragFloat("Velocity Size X", &miscConfig.plots.velocitySize.x, 1.f, 20.f, 400.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                ImGui::DragFloat("Velocity Size Y", &miscConfig.plots.velocitySize.y, 1.f, 10.f, 200.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    ImGuiCustom::colorPicker("Hit marker", miscConfig.hitMarker.hitMarker);
    miscConfig.hitMarker.hitMarker.thickness = std::clamp<float>(miscConfig.hitMarker.hitMarker.thickness, 0.f, 10.f);
    if (miscConfig.hitMarker.hitMarker.enabled) {
        ImGui::SameLine();
        ImGui::PushID("Hit marker");
        if (ImGui::Button("..."))
            ImGui::OpenPopup("HM");

        if (ImGui::BeginPopup("HM")) {
            float hitMarkerLength = miscConfig.hitMarker.hitMarkerLength + 4.f;
            if (ImGui::SliderFloat("Hit Marker Length", &hitMarkerLength, 1.f, 16.f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
                miscConfig.hitMarker.hitMarkerLength = hitMarkerLength - 4.f;
            ImGui::SliderFloat("Hit marker time", &miscConfig.hitMarker.hitMarkerTime, 0.1f, 1.5f, "%.2fs");
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    ImGuiCustom::colorPicker("Hit marker damage indicator", miscConfig.hitMarker.hitMarkerDamageIndicator);
    if (miscConfig.hitMarker.hitMarkerDamageIndicator.enabled) {
        ImGui::SameLine();
        ImGui::PushID("Hit marker damage indicator");
        if (ImGui::Button("..."))
            ImGui::OpenPopup("HMDI");

        if (ImGui::BeginPopup("HMDI")) {
            ImGui::Checkbox("Customize Hitmarker", &miscConfig.hitMarker.hitMarkerDamageIndicatorCustomize);
            if (miscConfig.hitMarker.hitMarkerDamageIndicatorCustomize) {
                ImGui::SliderInt(" ", &miscConfig.hitMarker.hitMarkerDamageIndicatorDist, 1, 100, "Dist: %d");
                ImGui::PushID(1);
                ImGui::SliderFloat(" ", &miscConfig.hitMarker.hitMarkerDamageIndicatorRatio, 0.1f, 1.0f, "Ratio: %.2f");
                ImGui::PopID();
            };
            ImGui::EndPopup();
        }
        ImGui::PopID();
    };

#ifdef _WIN32
    bool radioSwitched = ((radioStream == NULL) && miscConfig.radio.station);
    if (radioSwitched) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    }
    if (ImGui::Combo("Radio", &miscConfig.radio.station, radioNames.data(), radioNames.size()))
        if (miscConfig.radio.station)
            updateRadio();
        else
            updateRadio(true);
    if (radioSwitched) {
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    }
    if (miscConfig.radio.station) {
        ImGui::SliderFloat("Radio Volume", &miscConfig.radio.volume, 0.f, 100.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::Checkbox(miscConfig.radio.mute ? "Radio Muted" : "Radio Mute", &miscConfig.radio.mute);
    }
#endif

    if (ImGui::TreeNode("Style Configuration")) {
        if (ImGui::Combo("Menu colors", &miscConfig.menuColors,
            "Dark\0"
            "Light\0"
            "Classic\0"
            "Custom\0"
            "Custom (Easy)\0"
            "Fatality\0"
            "OneTap-V3\0"
            "Mutiny (WIP)\0"
            "CSGOSimple (WIP)\0"
            "CSGOSimple(Extender) (WIP)\0"
            "BoyNextHook (WIP)\0"
            "NIXWARE (WIP)\0"
            "Setupim (WIP)\0"
            "Monochrome (WIP)\0"
            "Procedural (WIP)\0"
            "Pagghiu (WIP)\0"
            "Doug (WIP)\0"
            "Microsoft (WIP)\0"
            "Darcula (WIP)\0"
            "UE4 (WIP)\0"
            "Cherry (WIP)\0"
            "LightGreen (WIP)\0"
            "Photoshop\0"
            "CorporateGrey\0"
            "Violet\0"
            "Raikiri\0"
            "VGUI\0"
            "Gold & Black\0"
            "Sewer\0"
            "VS (WIP)\0"
            "OverShifted\0"
            "RayTeak\0"))
            updateColors();

        if (miscConfig.menuColors == 3) {
            ImGuiStyle& style = ImGui::GetStyle();
            for (int i = 0; i < ImGuiCol_COUNT; i++)
                ImGuiCustom::colorPopup(ImGui::GetStyleColorName(i), (std::array<float, 4>&)style.Colors[i]);
        }
        // Custom (Easy)
        else if (miscConfig.menuColors == 4) {

            ImGuiStyle& style = ImGui::GetStyle();

            miscConfig.customEasy.BackGroundColor.color[0] = style.Colors[ImGuiCol_WindowBg].x;
            miscConfig.customEasy.BackGroundColor.color[1] = style.Colors[ImGuiCol_WindowBg].y;
            miscConfig.customEasy.BackGroundColor.color[2] = style.Colors[ImGuiCol_WindowBg].z;
            miscConfig.customEasy.TextColor.color[0] = style.Colors[ImGuiCol_Text].x;
            miscConfig.customEasy.TextColor.color[1] = style.Colors[ImGuiCol_Text].y;
            miscConfig.customEasy.TextColor.color[2] = style.Colors[ImGuiCol_Text].z;
            miscConfig.customEasy.MainColor.color[0] = style.Colors[ImGuiCol_Button].x;
            miscConfig.customEasy.MainColor.color[1] = style.Colors[ImGuiCol_Button].y;
            miscConfig.customEasy.MainColor.color[2] = style.Colors[ImGuiCol_Button].z;
            miscConfig.customEasy.MainAccentColor.color[0] = style.Colors[ImGuiCol_Header].x;
            miscConfig.customEasy.MainAccentColor.color[1] = style.Colors[ImGuiCol_Header].y;
            miscConfig.customEasy.MainAccentColor.color[2] = style.Colors[ImGuiCol_Header].z;
            miscConfig.customEasy.HighlightColor.color[0] = style.Colors[ImGuiCol_CheckMark].x;
            miscConfig.customEasy.HighlightColor.color[1] = style.Colors[ImGuiCol_CheckMark].y;
            miscConfig.customEasy.HighlightColor.color[2] = style.Colors[ImGuiCol_CheckMark].z;

            ImGui::Text("Go to \"coolors.co\" to generate beautiful palettes");
            ImGuiCustom::colorPopup("Background Color", miscConfig.customEasy.BackGroundColor.color, 0, 0, 0);
            ImGuiCustom::colorPopup("Text Color", miscConfig.customEasy.TextColor.color, 0, 0, 0);
            ImGuiCustom::colorPopup("Main Color", miscConfig.customEasy.MainColor.color, 0, 0, 0);
            ImGuiCustom::colorPopup("Main Accent Color", miscConfig.customEasy.MainAccentColor.color, 0, 0, 0);
            ImGuiCustom::colorPopup("Highlight Color", miscConfig.customEasy.HighlightColor.color, 0, 0, 0);

            auto BackGroundColor = miscConfig.customEasy.BackGroundColor.color;
            auto TextColor = miscConfig.customEasy.TextColor.color;
            auto MainColor = miscConfig.customEasy.MainColor.color;
            auto MainAccentColor = miscConfig.customEasy.MainAccentColor.color;
            auto HighlightColor = miscConfig.customEasy.HighlightColor.color;

            style.WindowMenuButtonPosition = ImGuiDir_Right;

#define GET_COLOR(c, a) {c[0], c[1], c[2], (a * miscConfig.customEasy.c.color[3])};
#define DARKEN(c ,p) ImVec4((std::max)(0.f, c.x - 1.0f * p), (std::max)(0.f, c.y - 1.0f * p), (std::max)(0.f, c.z - 1.0f *p), c.w);
#define LIGHTEN(x, y, z, w, p) ImVec4((std::max)(0.f, x + 1.0f * p), (std::max)(0.f, y + 1.0f * p), (std::max)(0.f, z + 1.0f *p), w);
#define DISABLED(c) DARKEN(c, 0.6f);
#define HOVERED(c) LIGHTEN(c.x, c.y, c.z, c.w, 0.2f);
#define ACTIVE(c, a) LIGHTEN(c.x, c.y, c.z, a, 0.1f);
#define COLLAPSED(c) DARKEN(c, 0.2f);

            style.Colors[ImGuiCol_Text] = GET_COLOR(TextColor, 0.8f);
            style.Colors[ImGuiCol_TextDisabled] = DISABLED(style.Colors[ImGuiCol_Text]);
            style.Colors[ImGuiCol_WindowBg] = GET_COLOR(BackGroundColor, 0.8f);
            style.Colors[ImGuiCol_ChildBg] = { 0.f, 0.f, 0.f, 0.2f };
            style.Colors[ImGuiCol_PopupBg] = GET_COLOR(BackGroundColor, 0.9f);
            style.Colors[ImGuiCol_Border] = LIGHTEN(style.Colors[ImGuiCol_PopupBg].x, style.Colors[ImGuiCol_PopupBg].y, style.Colors[ImGuiCol_PopupBg].z, style.Colors[ImGuiCol_PopupBg].w, 0.4f);
            style.Colors[ImGuiCol_BorderShadow] = { 0.f, 0.f, 0.f, 0.8f };
            style.Colors[ImGuiCol_FrameBg] = GET_COLOR(MainAccentColor, 0.4f);
            style.Colors[ImGuiCol_FrameBgHovered] = HOVERED(style.Colors[ImGuiCol_FrameBg]);
            style.Colors[ImGuiCol_FrameBgActive] = ACTIVE(style.Colors[ImGuiCol_FrameBg], (1.f * miscConfig.customEasy.MainAccentColor.color[3]));
            style.Colors[ImGuiCol_TitleBg] = style.Colors[ImGuiCol_WindowBg];
            style.Colors[ImGuiCol_TitleBgActive] = ACTIVE(style.Colors[ImGuiCol_TitleBg], (1.f * miscConfig.customEasy.BackGroundColor.color[3]));
            style.Colors[ImGuiCol_TitleBgCollapsed] = COLLAPSED(style.Colors[ImGuiCol_TitleBg]);
            style.Colors[ImGuiCol_MenuBarBg] = DARKEN(style.Colors[ImGuiCol_WindowBg], 0.2f);
            style.Colors[ImGuiCol_ScrollbarBg] = LIGHTEN(BackGroundColor[0], BackGroundColor[1], BackGroundColor[2], (0.5f * miscConfig.customEasy.BackGroundColor.color[3]), 0.4f);
            style.Colors[ImGuiCol_ScrollbarGrab] = LIGHTEN(style.Colors[ImGuiCol_WindowBg].x, style.Colors[ImGuiCol_WindowBg].y, style.Colors[ImGuiCol_WindowBg].z, style.Colors[ImGuiCol_WindowBg].w, 0.3f);
            style.Colors[ImGuiCol_ScrollbarGrabHovered] = HOVERED(style.Colors[ImGuiCol_ScrollbarGrab]);
            style.Colors[ImGuiCol_ScrollbarGrabActive] = ACTIVE(style.Colors[ImGuiCol_ScrollbarGrab], (1.f * miscConfig.customEasy.BackGroundColor.color[3]));
            style.Colors[ImGuiCol_CheckMark] = GET_COLOR(HighlightColor, 0.8f);
            style.Colors[ImGuiCol_SliderGrab] = style.Colors[ImGuiCol_CheckMark];
            style.Colors[ImGuiCol_SliderGrabActive] = ACTIVE(style.Colors[ImGuiCol_SliderGrab], (1.f * miscConfig.customEasy.HighlightColor.color[3]));
            style.Colors[ImGuiCol_Button] = GET_COLOR(MainColor, 0.8f);
            style.Colors[ImGuiCol_ButtonHovered] = HOVERED(style.Colors[ImGuiCol_Button]);
            style.Colors[ImGuiCol_ButtonActive] = ACTIVE(style.Colors[ImGuiCol_Button], (1.f * miscConfig.customEasy.MainColor.color[3]));
            style.Colors[ImGuiCol_Header] = GET_COLOR(MainAccentColor, 0.8f);
            style.Colors[ImGuiCol_HeaderHovered] = HOVERED(style.Colors[ImGuiCol_Header]);
            style.Colors[ImGuiCol_HeaderActive] = ACTIVE(style.Colors[ImGuiCol_Header], (1.f * miscConfig.customEasy.MainAccentColor.color[3]));
            style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
            style.Colors[ImGuiCol_SeparatorHovered] = HOVERED(style.Colors[ImGuiCol_Separator]);
            style.Colors[ImGuiCol_SeparatorActive] = ACTIVE(style.Colors[ImGuiCol_Separator], (1.f * miscConfig.customEasy.BackGroundColor.color[3]));
            style.Colors[ImGuiCol_ResizeGrip] = GET_COLOR(MainColor, 0.2f);
            style.Colors[ImGuiCol_ResizeGripHovered] = HOVERED(style.Colors[ImGuiCol_ResizeGrip]);
            style.Colors[ImGuiCol_ResizeGripActive] = ACTIVE(style.Colors[ImGuiCol_ResizeGrip], (1.f * miscConfig.customEasy.MainColor.color[3]));
            style.Colors[ImGuiCol_Tab] = GET_COLOR(MainColor, 0.6f);
            style.Colors[ImGuiCol_TabHovered] = HOVERED(style.Colors[ImGuiCol_Tab]);
            style.Colors[ImGuiCol_TabActive] = ACTIVE(style.Colors[ImGuiCol_Tab], (1.f * miscConfig.customEasy.MainColor.color[3]));
            style.Colors[ImGuiCol_TabUnfocused] = style.Colors[ImGuiCol_Tab];
            style.Colors[ImGuiCol_TabUnfocusedActive] = style.Colors[ImGuiCol_TabActive];
            style.Colors[ImGuiCol_PlotLines] = style.Colors[ImGuiCol_CheckMark];
            style.Colors[ImGuiCol_PlotLinesHovered] = HOVERED(style.Colors[ImGuiCol_PlotLines]);
            style.Colors[ImGuiCol_PlotHistogram] = style.Colors[ImGuiCol_CheckMark];
            style.Colors[ImGuiCol_PlotHistogramHovered] = HOVERED(style.Colors[ImGuiCol_PlotHistogram]);
            style.Colors[ImGuiCol_TextSelectedBg] = GET_COLOR(HighlightColor, 0.4f);
            style.Colors[ImGuiCol_DragDropTarget] = style.Colors[ImGuiCol_CheckMark];
            style.Colors[ImGuiCol_NavHighlight] = { 1.f, 1.f, 1.f, 0.8f };
            style.Colors[ImGuiCol_NavWindowingHighlight] = style.Colors[ImGuiCol_NavHighlight];
            style.Colors[ImGuiCol_NavWindowingDimBg] = { 1.f, 1.f, 1.f, 0.2f };
            style.Colors[ImGuiCol_ModalWindowDimBg] = { 1.f, 1.f, 1.f, 0.6f };

#undef GET_COLOR
#undef DARKEN
#undef LIGHTEN
#undef DISABLED
#undef HOVERED
#undef ACTIVE
#undef COLLAPSED
        }
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
            std::ranges::sort(playersOrdered, [](const auto& a, const auto& b) {
                // enemies first
                if (a.get().enemy != b.get().enemy)
                    return a.get().enemy && !b.get().enemy;

                return a.get().handle < b.get().handle;
                });

            ImGui::PushFont(gui->getUnicodeFont());

            for (const auto& player : playersOrdered) {
                ImGui::TableNextRow();
                ImGui::PushID(ImGui::TableGetRowIndex());

                ImGui::TableNextColumn();
                ImGui::textEllipsisInTableCell(player.get().name.c_str());

                if (ImGui::TableNextColumn() && ImGui::smallButtonFullWidth("Copy", player.get().steamID == 0))
                    ImGui::SetClipboardText(std::to_string(player.get().steamID).c_str());

                if (ImGui::TableNextColumn()) {
                    ImGui::Image(player.get().getRankTexture(), { 2.45f /* -> proportion 49x20px */ * ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight() });
                    if (ImGui::IsItemHovered()) {
                        ImGui::BeginTooltip();
                        ImGui::PushFont(nullptr);
                        ImGui::TextUnformatted(player.get().getRankName().c_str());
                        ImGui::PopFont();
                        ImGui::EndTooltip();
                    }
                }

                if (ImGui::TableNextColumn())
                    ImGui::Text("%d", player.get().competitiveWins);

                if (ImGui::TableNextColumn())
                    ImGui::TextColored({ 0.0f, 1.0f, 0.0f, 1.0f }, "$%d", player.get().money);

                if (ImGui::TableNextColumn()) {
                    if (!player.get().alive)
                        ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "%s", "DEAD");
                    else
                        ImGui::Text("%d HP", player.get().health);
                }

                if (ImGui::TableNextColumn())
                    ImGui::Text("%d", player.get().armor);

                if (ImGui::TableNextColumn())
                    ImGui::TextUnformatted(player.get().lastPlaceName.c_str());

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

            if (count < 1)
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

static void rainbowBar(ImDrawList* drawList)noexcept
{
    if (!miscConfig.rainbowBar.rainbowBar.enabled)
        return;

    float colorR = 0;
    float colorG = 0;
    float colorB = 0;
    if (miscConfig.rainbowBar.rainbowBar.rainbow) {
        colorR = std::sin(miscConfig.rainbowBar.rainbowBar.rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f;
        colorG = std::sin(miscConfig.rainbowBar.rainbowBar.rainbowSpeed * memory->globalVars->realtime + 2 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f;
        colorB = std::sin(miscConfig.rainbowBar.rainbowBar.rainbowSpeed * memory->globalVars->realtime + 4 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f;
    }
    else {
        colorR = miscConfig.rainbowBar.rainbowBar.color[0];
        colorG = miscConfig.rainbowBar.rainbowBar.color[1];
        colorB = miscConfig.rainbowBar.rainbowBar.color[2];
    }
    float colorA = miscConfig.rainbowBar.rainbowBar.color[3];
    float tickness = miscConfig.rainbowBar.rainbowBar.thickness;
    float scale = miscConfig.rainbowBar.rainbowScale;
    float pulse, pulseAlpha;
    if (miscConfig.rainbowBar.rainbowPulse) {
        pulse = std::sin(miscConfig.rainbowBar.rainbowPulseSpeed * memory->globalVars->realtime) * 0.5f + 0.5f;
        pulseAlpha = (std::sin(miscConfig.rainbowBar.rainbowPulseSpeed * memory->globalVars->realtime) * 0.5f + 0.5f) * colorA;
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
        miscConfig.rainbowBar.rainbowBar.thickness = ds.y;
        tickness = ds.y;
    }

    //drawList->AddRectFilledMultiColor(upper - left, lower - right, Color Upper Left, Color Upper Right, Color Bottom Right, Color Bottom Left);

    if (miscConfig.rainbowBar.rainbowBottom) {
        // Bottom
        drawList->AddRectFilledMultiColor({ zero.x, ds.y - tickness }, { ds.x / 2, ds.y }, indigo0, blue0, blue, indigo);
        drawList->AddRectFilledMultiColor({ ds.x / 2, ds.y - tickness }, { ds.x, ds.y }, blue0, cyan0, cyan, blue);
    }
    if (miscConfig.rainbowBar.rainbowLeft) {
        // Left
        drawList->AddRectFilledMultiColor(zero, { tickness, ds.y / 2 }, red, red0, magenta0, magenta);
        drawList->AddRectFilledMultiColor({ zero.x, ds.y / 2 }, { tickness, ds.y }, magenta, magenta0, indigo0, indigo);
    }
    if (miscConfig.rainbowBar.rainbowRight) {
        // Right
        drawList->AddRectFilledMultiColor({ ds.x - tickness, zero.y }, { ds.x, ds.y / 2 }, chartreuse0, chartreuse, malachite, malachite0);
        drawList->AddRectFilledMultiColor({ ds.x - tickness, ds.y / 2 }, ds, malachite0, malachite, cyan, cyan0);
    }
    if (miscConfig.rainbowBar.rainbowUp) {
        // Upper
        drawList->AddRectFilledMultiColor(zero, { ds.x / 2, tickness + (0.0f) }, red, amber, amber0, red0);
        drawList->AddRectFilledMultiColor({ ds.x / 2, zero.y }, { ds.x, tickness + (0.0f) }, amber, chartreuse, chartreuse0, amber0);
    }
}

static void watermark() noexcept
{
    if (miscConfig.watermark.watermark.enabled) {
        std::string watermark = "GOESP BETA";

        if (interfaces->engine->isInGame() && miscConfig.watermark.watermarkNickname) {
            PlayerInfo playerInfo;
            auto nickname = interfaces->engine->getPlayerInfo(localPlayer->index(), playerInfo);
            watermark.append(" | ").append(playerInfo.name);
        };

        if (miscConfig.watermark.watermarkUsername)
            watermark.append(" | ")
#ifdef _WIN32
            .append(getenv("USERNAME"));
#else
            .append(getenv("USER"));
#endif

        if (miscConfig.watermark.watermarkFPS) {
            static auto frameRate = 1.0f;
            frameRate = 0.9f * frameRate + 0.1f * memory->globalVars->absoluteFrameTime;
            watermark.append(" | FPS: ").append(std::to_string(static_cast<int>(1 / frameRate)));
        }

        if (miscConfig.watermark.watermarkPing) {
            float latency = 0.0f;
            if (auto networkChannel = interfaces->engine->getNetworkChannel(); networkChannel && networkChannel->getLatency(0) > 0.0f)
                latency = networkChannel->getLatency(0);
            watermark.append(" | Ping: ").append(std::to_string(static_cast<int>(latency * 1000))).append(" ms");
        }

        if (miscConfig.watermark.watermarkTickrate)
            watermark.append(" | ").append(std::to_string(static_cast<int>(1.0f / memory->globalVars->intervalPerTick))).append(" tick");

        if (miscConfig.watermark.watermarkVelocity && localPlayer && localPlayer->isAlive())
            watermark.append(" | ").append(std::to_string(static_cast<int>(round(localPlayer->velocity().length2D())))).append(" ups");

        if (miscConfig.watermark.watermarkTime) {
            const auto time = std::time(nullptr);
            const auto localTime = std::localtime(&time);
            std::ostringstream timeShow;
            timeShow << std::setfill('0') << std::setw(2) << localTime->tm_hour << ":" << std::setw(2) << localTime->tm_min << ":" << std::setw(2) << localTime->tm_sec;
            watermark.append(" | ").append(timeShow.str());
        }

        auto pos = miscConfig.watermark.watermarkPos * ImGui::GetIO().DisplaySize;
        ImGuiCond nextFlag = ImGuiCond_None;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f }, ImGuiCond_Always);
        if (ImGui::IsMouseDown(0))
            nextFlag |= ImGuiCond_Once;
        else
            nextFlag |= ImGuiCond_Always;
        ImGui::SetNextWindowPos(pos, nextFlag);

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        if (!gui->isOpen())
            windowFlags |= ImGuiWindowFlags_NoInputs;

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, miscConfig.watermark.watermark.color[3]);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, { 0.5f, 0.5f });
        ImGui::Begin("Watermark", nullptr, windowFlags);
        ImGui::PopStyleVar();
        ImGui::PushFont(gui->getUnicodeFont());

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
        miscConfig.watermark.watermarkPos = ImVec2{ x / ds.x ,y / ds.y };

        ImGui::SetWindowFontScale(miscConfig.watermark.watermarkScale);
        if (miscConfig.watermark.watermark.rainbow) {
            auto colorR = std::sin(miscConfig.watermark.watermark.rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f;
            auto colorG = std::sin(miscConfig.watermark.watermark.rainbowSpeed * memory->globalVars->realtime + 2 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f;
            auto colorB = std::sin(miscConfig.watermark.watermark.rainbowSpeed * memory->globalVars->realtime + 4 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f;
#ifndef _WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#endif
            ImGui::TextColored({ colorR, colorG, colorB, 1.f }, watermark.c_str());
#ifndef _WIN32
#pragma GCC diagnostic pop
#endif
        }
        else
#ifndef _WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#endif
            ImGui::TextColored({ miscConfig.watermark.watermark.color[0], miscConfig.watermark.watermark.color[1] ,miscConfig.watermark.watermark.color[2], 1.f }, watermark.c_str());
#ifndef _WIN32
#pragma GCC diagnostic pop
#endif

        ImGui::PopFont();
        ImGui::End();
    }
}

static void plots() noexcept
{
    if (miscConfig.plots.enabled)
    {
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        if (!gui->isOpen())
            windowFlags |= ImGuiWindowFlags_NoInputs;

        if (miscConfig.plots.FPS)
        {
            auto pos = miscConfig.plots.FPSPos * ImGui::GetIO().DisplaySize;
            ImGuiCond nextFlag = ImGuiCond_None;
            ImGui::SetNextWindowSize({ 0.0f, 0.0f }, ImGuiCond_Always);
            if (ImGui::IsMouseDown(0))
                nextFlag |= ImGuiCond_Once;
            else
                nextFlag |= ImGuiCond_Always;
            ImGui::SetNextWindowPos(pos, nextFlag);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, { 0.5f, 0.5f });
            if (miscConfig.plots.FPSStyle)
            {
                if (miscConfig.plots.FPSStyleLines.rainbow) {
                    ImVec4 rainbow = {
                        std::sin(miscConfig.plots.FPSStyleLines.rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.FPSStyleLines.rainbowSpeed * memory->globalVars->realtime + 2 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.FPSStyleLines.rainbowSpeed * memory->globalVars->realtime + 4 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        miscConfig.plots.FPSStyleLines.color[3] };
                    ImGui::PushStyleColor(ImGuiCol_PlotLines, rainbow);
                }
                else
                    ImGui::PushStyleColor(ImGuiCol_PlotLines, miscConfig.plots.FPSStyleLines.color);
                if (miscConfig.plots.FPSStyleLinesHovered.rainbow) {
                    ImVec4 rainbow = {
                        std::sin(miscConfig.plots.FPSStyleLinesHovered.rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.FPSStyleLinesHovered.rainbowSpeed * memory->globalVars->realtime + 2 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.FPSStyleLinesHovered.rainbowSpeed * memory->globalVars->realtime + 4 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        miscConfig.plots.FPSStyleLinesHovered.color[3] };
                    ImGui::PushStyleColor(ImGuiCol_PlotLinesHovered, rainbow);
                }
                else
                    ImGui::PushStyleColor(ImGuiCol_PlotLinesHovered, miscConfig.plots.FPSStyleLinesHovered.color);
                if (miscConfig.plots.FPSStyleHistogram.rainbow) {
                    ImVec4 rainbow = {
                        std::sin(miscConfig.plots.FPSStyleHistogram.rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.FPSStyleHistogram.rainbowSpeed * memory->globalVars->realtime + 2 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.FPSStyleHistogram.rainbowSpeed * memory->globalVars->realtime + 4 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        miscConfig.plots.FPSStyleHistogram.color[3] };
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, rainbow);
                }
                else
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, miscConfig.plots.FPSStyleHistogram.color);
                if (miscConfig.plots.FPSStyleHistogramHovered.rainbow) {
                    ImVec4 rainbow = {
                        std::sin(miscConfig.plots.FPSStyleHistogramHovered.rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.FPSStyleHistogramHovered.rainbowSpeed * memory->globalVars->realtime + 2 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.FPSStyleHistogramHovered.rainbowSpeed * memory->globalVars->realtime + 4 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        miscConfig.plots.FPSStyleHistogramHovered.color[3] };
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogramHovered, rainbow);
                }
                else
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogramHovered, miscConfig.plots.FPSStyleHistogramHovered.color);
            }
            ImGui::Begin("FPS Plot", nullptr, windowFlags);
            if (miscConfig.plots.FPSStyle)
                ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::PushFont(gui->getUnicodeFont());

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
            miscConfig.plots.FPSPos = ImVec2{ x / ds.x ,y / ds.y };

            ImGui::SetWindowFontScale(miscConfig.plots.FPSScale);

            static std::array<float, 90> fps{};
            static int fps_offset = 0;
            static double refresh_time = 0.;
            if (refresh_time == 0.)
                refresh_time = ImGui::GetTime();
            while (refresh_time < ImGui::GetTime())
            {
                static auto frameRate = 1.f;
                frameRate = 0.9f * frameRate + 0.1f * memory->globalVars->absoluteFrameTime;
                fps.at(fps_offset) = 1 / frameRate;
                fps_offset = (fps_offset + 1) % fps.size();
                refresh_time += 1.f / miscConfig.plots.FPSRefresh;
            }

            {
                float average = 0.f;
                for (size_t n = 0; n < fps.size(); n++)
                    average += fps.at(n);
                average /= static_cast<float>(fps.size());
                float max = *std::max_element(fps.begin(), fps.end());
                float min = *std::min_element(fps.begin(), fps.end());
                std::stringstream overlay;
                if (miscConfig.plots.FPSInfo == 1)
                    overlay << "FPS";
                else if (miscConfig.plots.FPSInfo == 2)
                    overlay << "FPS" << std::endl <<
                    "max: " << std::setprecision(2) << std::fixed << max << std::endl <<
                    "min: " << std::setprecision(2) << std::fixed << min << std::endl <<
                    "avg: " << std::setprecision(2) << std::fixed << average;
                bool info = miscConfig.plots.FPSInfo != 0;
                if (miscConfig.plots.FPS == 1)
                    ImGui::PlotLines(info ? overlay.str().c_str() : "", fps.data(), fps.size(), fps_offset, nullptr, 0.f, max, miscConfig.plots.FPSSize);
                else if (miscConfig.plots.FPS == 2)
                    ImGui::PlotHistogram(info ? overlay.str().c_str() : "", fps.data(), fps.size(), fps_offset, nullptr, 0.f, max, miscConfig.plots.FPSSize);
            }

            ImGui::PopFont();
            ImGui::End();
        }
        if (miscConfig.plots.ping)
        {
            auto pos = miscConfig.plots.pingPos * ImGui::GetIO().DisplaySize;
            ImGuiCond nextFlag = ImGuiCond_None;
            ImGui::SetNextWindowSize({ 0.0f, 0.0f }, ImGuiCond_Always);
            if (ImGui::IsMouseDown(0))
                nextFlag |= ImGuiCond_Once;
            else
                nextFlag |= ImGuiCond_Always;
            ImGui::SetNextWindowPos(pos, nextFlag);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, { 0.5f, 0.5f });
            if (miscConfig.plots.pingStyle)
            {
                if (miscConfig.plots.pingStyleLines.rainbow) {
                    ImVec4 rainbow = {
                        std::sin(miscConfig.plots.pingStyleLines.rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.pingStyleLines.rainbowSpeed * memory->globalVars->realtime + 2 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.pingStyleLines.rainbowSpeed * memory->globalVars->realtime + 4 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        miscConfig.plots.pingStyleLines.color[3] };
                    ImGui::PushStyleColor(ImGuiCol_PlotLines, rainbow);
                }
                else
                    ImGui::PushStyleColor(ImGuiCol_PlotLines, miscConfig.plots.pingStyleLines.color);
                if (miscConfig.plots.pingStyleLinesHovered.rainbow) {
                    ImVec4 rainbow = {
                        std::sin(miscConfig.plots.pingStyleLinesHovered.rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.pingStyleLinesHovered.rainbowSpeed * memory->globalVars->realtime + 2 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.pingStyleLinesHovered.rainbowSpeed * memory->globalVars->realtime + 4 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        miscConfig.plots.pingStyleLinesHovered.color[3] };
                    ImGui::PushStyleColor(ImGuiCol_PlotLinesHovered, rainbow);
                }
                else
                    ImGui::PushStyleColor(ImGuiCol_PlotLinesHovered, miscConfig.plots.pingStyleLinesHovered.color);
                if (miscConfig.plots.pingStyleHistogram.rainbow) {
                    ImVec4 rainbow = {
                        std::sin(miscConfig.plots.pingStyleHistogram.rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.pingStyleHistogram.rainbowSpeed * memory->globalVars->realtime + 2 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.pingStyleHistogram.rainbowSpeed * memory->globalVars->realtime + 4 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        miscConfig.plots.pingStyleHistogram.color[3] };
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, rainbow);
                }
                else
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, miscConfig.plots.pingStyleHistogram.color);
                if (miscConfig.plots.pingStyleHistogramHovered.rainbow) {
                    ImVec4 rainbow = {
                        std::sin(miscConfig.plots.pingStyleHistogramHovered.rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.pingStyleHistogramHovered.rainbowSpeed * memory->globalVars->realtime + 2 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.pingStyleHistogramHovered.rainbowSpeed * memory->globalVars->realtime + 4 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        miscConfig.plots.pingStyleHistogramHovered.color[3] };
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogramHovered, rainbow);
                }
                else
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogramHovered, miscConfig.plots.pingStyleHistogramHovered.color);
            }
            ImGui::Begin("Ping Plot", nullptr, windowFlags);
            if (miscConfig.plots.pingStyle)
                ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::PushFont(gui->getUnicodeFont());

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
            miscConfig.plots.pingPos = ImVec2{ x / ds.x ,y / ds.y };

            ImGui::SetWindowFontScale(miscConfig.plots.pingScale);

            static std::array<float, 90> ping{};
            static int ping_offset = 0;
            static double refresh_time = 0.;
            if (refresh_time == 0.)
                refresh_time = ImGui::GetTime();
            while (refresh_time < ImGui::GetTime())
            {
                float latency = 0.0f;
                if (auto networkChannel = interfaces->engine->getNetworkChannel(); networkChannel && networkChannel->getLatency(0) > 0.0f)
                    latency = networkChannel->getLatency(0);
                ping.at(ping_offset) = latency * 1000;
                ping_offset = (ping_offset + 1) % ping.size();
                refresh_time += 1.f / miscConfig.plots.pingRefresh;
            }

            {
                float average = 0.f;
                for (size_t n = 0; n < ping.size(); n++)
                    average += ping.at(n);
                average /= static_cast<float>(ping.size());
                float max = *std::max_element(ping.begin(), ping.end());
                float min = *std::min_element(ping.begin(), ping.end());
                std::stringstream overlay;
                if (miscConfig.plots.pingInfo == 1)
                    overlay << "Ping";
                else if (miscConfig.plots.pingInfo == 2)
                    overlay << "Ping" << std::endl <<
                    "max: " << std::setprecision(2) << std::fixed << max << std::endl <<
                    "min: " << std::setprecision(2) << std::fixed << min << std::endl <<
                    "avg: " << std::setprecision(2) << std::fixed << average;
                bool info = miscConfig.plots.pingInfo != 0;
                if (miscConfig.plots.ping == 1)
                    ImGui::PlotLines(info ? overlay.str().c_str() : "", ping.data(), ping.size(), ping_offset, nullptr, 0.f, max, miscConfig.plots.pingSize);
                else if (miscConfig.plots.ping == 2)
                    ImGui::PlotHistogram(info ? overlay.str().c_str() : "", ping.data(), ping.size(), ping_offset, nullptr, 0.f, max, miscConfig.plots.pingSize);
            }

            ImGui::PopFont();
            ImGui::End();
        }
        if (miscConfig.plots.velocity)
        {
            auto pos = miscConfig.plots.velocityPos * ImGui::GetIO().DisplaySize;
            ImGuiCond nextFlag = ImGuiCond_None;
            ImGui::SetNextWindowSize({ 0.0f, 0.0f }, ImGuiCond_Always);
            if (ImGui::IsMouseDown(0))
                nextFlag |= ImGuiCond_Once;
            else
                nextFlag |= ImGuiCond_Always;
            ImGui::SetNextWindowPos(pos, nextFlag);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, { 0.5f, 0.5f });
            if (miscConfig.plots.velocityStyle)
            {
                if (miscConfig.plots.velocityStyleLines.rainbow) {
                    ImVec4 rainbow = {
                        std::sin(miscConfig.plots.velocityStyleLines.rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.velocityStyleLines.rainbowSpeed * memory->globalVars->realtime + 2 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.velocityStyleLines.rainbowSpeed * memory->globalVars->realtime + 4 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        miscConfig.plots.velocityStyleLines.color[3] };
                    ImGui::PushStyleColor(ImGuiCol_PlotLines, rainbow);
                }
                else
                    ImGui::PushStyleColor(ImGuiCol_PlotLines, miscConfig.plots.velocityStyleLines.color);
                if (miscConfig.plots.velocityStyleLinesHovered.rainbow) {
                    ImVec4 rainbow = {
                        std::sin(miscConfig.plots.velocityStyleLinesHovered.rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.velocityStyleLinesHovered.rainbowSpeed * memory->globalVars->realtime + 2 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.velocityStyleLinesHovered.rainbowSpeed * memory->globalVars->realtime + 4 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        miscConfig.plots.velocityStyleLinesHovered.color[3] };
                    ImGui::PushStyleColor(ImGuiCol_PlotLinesHovered, rainbow);
                }
                else
                    ImGui::PushStyleColor(ImGuiCol_PlotLinesHovered, miscConfig.plots.velocityStyleLinesHovered.color);
                if (miscConfig.plots.velocityStyleHistogram.rainbow) {
                    ImVec4 rainbow = {
                        std::sin(miscConfig.plots.velocityStyleHistogram.rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.velocityStyleHistogram.rainbowSpeed * memory->globalVars->realtime + 2 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.velocityStyleHistogram.rainbowSpeed * memory->globalVars->realtime + 4 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        miscConfig.plots.velocityStyleHistogram.color[3] };
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, rainbow);
                }
                else
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, miscConfig.plots.velocityStyleHistogram.color);
                if (miscConfig.plots.velocityStyleHistogramHovered.rainbow) {
                    ImVec4 rainbow = {
                        std::sin(miscConfig.plots.velocityStyleHistogramHovered.rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.velocityStyleHistogramHovered.rainbowSpeed * memory->globalVars->realtime + 2 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        std::sin(miscConfig.plots.velocityStyleHistogramHovered.rainbowSpeed * memory->globalVars->realtime + 4 * std::numbers::pi_v<float> / 3) * 0.5f + 0.5f,
                        miscConfig.plots.velocityStyleHistogramHovered.color[3] };
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogramHovered, rainbow);
                }
                else
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogramHovered, miscConfig.plots.velocityStyleHistogramHovered.color);
            }
            ImGui::Begin("Velocity Plot", nullptr, windowFlags);
            if (miscConfig.plots.velocityStyle)
                ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::PushFont(gui->getUnicodeFont());

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
            miscConfig.plots.velocityPos = ImVec2{ x / ds.x ,y / ds.y };

            ImGui::SetWindowFontScale(miscConfig.plots.velocityScale);

            static std::array<float, 90> velocity{};
            static int velocity_offset = 0;
            static double refresh_time = 0.;
            if (refresh_time == 0.)
                refresh_time = ImGui::GetTime();
            while (refresh_time < ImGui::GetTime())
            {
                float velocityVal = 0.f;
                if (localPlayer && localPlayer->isAlive())
                    velocityVal = localPlayer->velocity().length2D();
                velocity.at(velocity_offset) = velocityVal;
                velocity_offset = (velocity_offset + 1) % velocity.size();
                refresh_time += 1.f / miscConfig.plots.velocityRefresh;
            }

            {
                float average = 0.f;
                for (size_t n = 0; n < velocity.size(); n++)
                    average += velocity.at(n);
                average /= static_cast<float>(velocity.size());
                float max = *std::max_element(velocity.begin(), velocity.end());
                float min = *std::min_element(velocity.begin(), velocity.end());
                std::stringstream overlay;
                if (miscConfig.plots.velocityInfo == 1)
                    overlay << "Velocity";
                else if (miscConfig.plots.velocityInfo == 2)
                    overlay << "Velocity" << std::endl <<
                    "max: " << std::setprecision(2) << std::fixed << max << std::endl <<
                    "min: " << std::setprecision(2) << std::fixed << min << std::endl <<
                    "avg: " << std::setprecision(2) << std::fixed << average;
                bool info = miscConfig.plots.velocityInfo != 0;
                if (miscConfig.plots.velocity == 1)
                    ImGui::PlotLines(info ? overlay.str().c_str() : "", velocity.data(), velocity.size(), velocity_offset, nullptr, 0.f, max, miscConfig.plots.velocitySize);
                else if (miscConfig.plots.velocity == 2)
                    ImGui::PlotHistogram(info ? overlay.str().c_str() : "", velocity.data(), velocity.size(), velocity_offset, nullptr, 0.f, max, miscConfig.plots.velocitySize);
            }

            ImGui::PopFont();
            ImGui::End();
        }
    }
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

    auto start = 4;
    const auto width_mid = ds.x / 2;
    const auto height_mid = ds.y / 2;

    auto drawList = ImGui::GetBackgroundDrawList();
    ImU32 color = Helpers::calculateColor(miscConfig.hitMarker.hitMarker);
    drawList->AddLine({ width_mid + miscConfig.hitMarker.hitMarkerLength, height_mid + miscConfig.hitMarker.hitMarkerLength }, { width_mid + start, height_mid + start }, color, miscConfig.hitMarker.hitMarker.thickness);
    drawList->AddLine({ width_mid - miscConfig.hitMarker.hitMarkerLength, height_mid + miscConfig.hitMarker.hitMarkerLength }, { width_mid - start, height_mid + start }, color, miscConfig.hitMarker.hitMarker.thickness);
    drawList->AddLine({ width_mid + miscConfig.hitMarker.hitMarkerLength, height_mid - miscConfig.hitMarker.hitMarkerLength }, { width_mid + start, height_mid - start }, color, miscConfig.hitMarker.hitMarker.thickness);
    drawList->AddLine({ width_mid - miscConfig.hitMarker.hitMarkerLength, height_mid - miscConfig.hitMarker.hitMarkerLength }, { width_mid - start, height_mid - start }, color, miscConfig.hitMarker.hitMarker.thickness);
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

    if (miscConfig.hitMarker.hitMarkerDamageIndicator.enabled)
        if (event && interfaces->engine->getPlayerForUserId(event->getInt("attacker")) == localPlayer->index())
            hitMarkerInfo.push_back({ memory->globalVars->realtime + miscConfig.hitMarker.hitMarkerTime, event->getInt("dmg_health") });
}

static void hitMarkerDamageIndicator() noexcept
{
    if (miscConfig.hitMarker.hitMarkerDamageIndicator.enabled) {
        if (hitMarkerInfo.empty()) return;

        const auto ds = ImGui::GetIO().DisplaySize;

        ImGui::PushFont(gui->getUnicodeFont());
        for (size_t i = 0; i < hitMarkerInfo.size(); i++) {
            const auto diff = hitMarkerInfo.at(i).hitMarkerExpTime - memory->globalVars->realtime;

            if (diff < 0.f) {
                hitMarkerInfo.erase(hitMarkerInfo.begin() + i);
                continue;
            }

            const auto dist = miscConfig.hitMarker.hitMarkerDamageIndicatorCustomize ? miscConfig.hitMarker.hitMarkerDamageIndicatorDist : 50;
            const auto ratio = (miscConfig.hitMarker.hitMarkerDamageIndicatorCustomize ? miscConfig.hitMarker.hitMarkerDamageIndicatorRatio : 0.6f) - diff;

            auto drawList = ImGui::GetBackgroundDrawList();
            ImU32 color = Helpers::calculateColor(miscConfig.hitMarker.hitMarkerDamageIndicator);
            drawList->AddText({ ds.x / 2 + (miscConfig.hitMarker.hitMarker.enabled ? miscConfig.hitMarker.hitMarkerLength + 2 : 2) + ratio * dist / 2, ds.y / 2 + (miscConfig.hitMarker.hitMarker.enabled ? miscConfig.hitMarker.hitMarkerLength + 2 : 2) + ratio * dist }, color, std::to_string(hitMarkerInfo.at(i).hitMarkerDmg).c_str());
        }
        ImGui::PopFont();
    }
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

    rainbowBar(drawList);
    watermark();
    plots();
    hitMarker();
    hitMarkerDamageIndicator();
#ifdef _WIN32
    radio();
#endif
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
    to_json(j, static_cast<const Color&>(o));

    WRITE("Enabled", enabled)
    WRITE("Audible Only", audibleOnly)
    WRITE("Spotted Only", spottedOnly)
}

static void to_json(json& j, const PlayerList& o, const PlayerList& dummy = {})
{
    WRITE("Enabled", enabled)
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

static void to_json(json& j, const RainbowBar& o, const RainbowBar& dummy = {})
{
    WRITE_OBJ("Rainbow Bar", rainbowBar);
    WRITE("Rainbow Up", rainbowUp);
    WRITE("Rainbow Bottom", rainbowBottom);
    WRITE("Rainbow Left", rainbowLeft);
    WRITE("Rainbow Right", rainbowRight);
    WRITE("Rainbow Scale", rainbowScale);
    WRITE("Rainbow Pulse", rainbowPulse);
    WRITE("Rainbow Pulse Speed", rainbowPulseSpeed);
}

static void to_json(json& j, const StyleCustomEasy& o, const StyleCustomEasy& dummy = {})
{
    WRITE_OBJ("Background Color", BackGroundColor);
    WRITE_OBJ("Text Color", TextColor);
    WRITE_OBJ("Main Color", MainColor);
    WRITE_OBJ("Main Accent Color", MainAccentColor);
    WRITE_OBJ("Highlight Color", HighlightColor);
}

static void to_json(json& j, const ImVec4& o)
{
    j[0] = o.x;
    j[1] = o.y;
    j[2] = o.z;
    j[3] = o.w;
}

static void to_json(json& j, const Watermark& o, const Watermark& dummy = {})
{
    WRITE_OBJ("Watermark", watermark);
    WRITE("Watermark Nickname", watermarkNickname);
    WRITE("Watermark Username", watermarkUsername);
    WRITE("Watermark FPS", watermarkFPS);
    WRITE("Watermark Ping", watermarkPing);
    WRITE("Watermark Tickrate", watermarkTickrate);
    WRITE("Watermark Velocity", watermarkVelocity);
    WRITE("Watermark Time", watermarkTime);
    WRITE("Watermark Alpha", watermarkAlpha);
    WRITE_OBJ("Watermark Pos", watermarkPos);
    WRITE("Watermark Scale", watermarkScale);
}

static void to_json(json& j, const Plots& o, const Plots& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("FPS Mode", FPS);
    WRITE("FPS Refrash Rate", FPSRefresh);
    WRITE("FPS Info mode", FPSInfo);
    WRITE("FPS Style switch", FPSStyle);
    WRITE_OBJ("FPS Style Lines", FPSStyleLines);
    WRITE_OBJ("FPS Style Lines Hovered", FPSStyleLinesHovered);
    WRITE_OBJ("FPS Style Histogram", FPSStyleHistogram);
    WRITE_OBJ("FPS Style Histogram Hovered", FPSStyleHistogramHovered);
    WRITE("FPS Pos", FPSPos);
    WRITE("FPS Size", FPSSize);
    WRITE("FPS Scale", FPSScale);
    WRITE("Ping Mode", ping);
    WRITE("Ping Refrash Rate", pingRefresh);
    WRITE("Ping Info mode", pingInfo);
    WRITE("Ping Style switch", pingStyle);
    WRITE_OBJ("Ping Style Lines", pingStyleLines);
    WRITE_OBJ("Ping Style Lines Hovered", pingStyleLinesHovered);
    WRITE_OBJ("Ping Style Histogram", pingStyleHistogram);
    WRITE_OBJ("Ping Style Histogram Hovered", pingStyleHistogramHovered);
    WRITE("Ping Pos", pingPos);
    WRITE("Ping Size", pingSize);
    WRITE("Ping Scale", pingScale);
    WRITE("Velocity Mode", velocity);
    WRITE("Velocity Refrash Rate", velocityRefresh);
    WRITE("Velocity Info mode", velocityInfo);
    WRITE("Velocity Style switch", velocityStyle);
    WRITE_OBJ("Velocity Style Lines", velocityStyleLines);
    WRITE_OBJ("Velocity Style Lines Hovered", velocityStyleLinesHovered);
    WRITE_OBJ("Velocity Style Histogram", velocityStyleHistogram);
    WRITE_OBJ("Velocity Style Histogram Hovered", velocityStyleHistogramHovered);
    WRITE("Velocity Pos", velocityPos);
    WRITE("Velocity Size", velocitySize);
    WRITE("Velocity Scale", velocityScale);
}

static void to_json(json& j, const HitMarker& o, const HitMarker& dummy = {})
{
    WRITE_OBJ("HitMarker", hitMarker);
    WRITE("HitMarker Length", hitMarkerLength);
    WRITE("HitMarker Time", hitMarkerTime);
    WRITE_OBJ("Hit marker damage indicator", hitMarkerDamageIndicator);
    WRITE("Hit marker damage indicator Customize", hitMarkerDamageIndicatorCustomize);
    WRITE("Hit marker damage indicator Dist", hitMarkerDamageIndicatorDist);
    WRITE("Hit marker damage indicator Ratio", hitMarkerDamageIndicatorRatio);
}

#ifdef _WIN32
static void to_json(json& j, const Radio& o, const Radio& dummy = {})
{
    WRITE("Radio Station", station);
    WRITE("Radio Volume", volume);
    WRITE("Radio Mute", mute);
}
#endif

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

    WRITE_OBJ("Rainbow Bar", rainbowBar);
    WRITE_OBJ("Watermark", watermark);
    WRITE_OBJ("Plots", plots);
    WRITE_OBJ("HitMarker", hitMarker);
#ifdef _WIN32
    WRITE_OBJ("Radio", radio);
#endif

    WRITE("Menu Color", menuColors);
    {
        auto& colors = j["Colors"];
        ImGuiStyle& style = ImGui::GetStyle();
        for (int i = 0; i < ImGuiCol_COUNT; i++)
            colors[ImGui::GetStyleColorName(i)] = style.Colors[i];
    }
    WRITE_OBJ("Menu color Custom (Easy)", customEasy);

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
    from_json(j, static_cast<Color&>(o));

    read(j, "Enabled", o.enabled);
    read(j, "Audible Only", o.audibleOnly);
    read(j, "Spotted Only", o.spottedOnly);
}

static void from_json(const json& j, PlayerList& o)
{
    read(j, "Enabled", o.enabled);
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

static void from_json(const json& j, RainbowBar& o)
{
    read_number(j, "Rainbow Bar Tickness", o.rainbowBar.thickness);
    read(j, "Rainbow Bar Color", o.rainbowBar.color);
    read(j, "Rainbow Bar Rainbow Switch", o.rainbowBar.rainbow);
    read_number(j, "Rainbow Bar Rainbow Speed", o.rainbowBar.rainbowSpeed);
    read(j, "Rainbow Bar Swith", o.rainbowBar.enabled);
    read(j, "Rainbow Up", o.rainbowUp);
    read(j, "Rainbow Bottom", o.rainbowBottom);
    read(j, "Rainbow Left", o.rainbowLeft);
    read(j, "Rainbow Right", o.rainbowRight);
    read_number(j, "Rainbow Scale", o.rainbowScale);
    read(j, "Rainbow Pulse", o.rainbowPulse);
    read_number(j, "Rainbow Pulse Speed", o.rainbowPulseSpeed);
}

static void from_json(const json& j, StyleCustomEasy& o)
{
    read<value_t::object>(j, "Background Color", o.BackGroundColor);
    read<value_t::object>(j, "Text Color", o.TextColor);
    read<value_t::object>(j, "Main Color", o.MainColor);
    read<value_t::object>(j, "Main Accent Color", o.MainAccentColor);
    read<value_t::object>(j, "Highlight Color", o.HighlightColor);
}

static void from_json(const json& j, ImVec4& o)
{
    o.x = j[0];
    o.y = j[1];
    o.z = j[2];
    o.w = j[3];
}

static void from_json(const json& j, Watermark& o)
{
    read<value_t::object>(j, "Watermark", o.watermark);
    read(j, "Watermark Nickname", o.watermarkNickname);
    read(j, "Watermark Username", o.watermarkUsername);
    read(j, "Watermark FPS", o.watermarkFPS);
    read(j, "Watermark Ping", o.watermarkPing);
    read(j, "Watermark Tickrate", o.watermarkTickrate);
    read(j, "Watermark Velocity", o.watermarkVelocity);
    read(j, "Watermark Time", o.watermarkTime);
    read_number(j, "Watermark Alpha", o.watermarkAlpha);
    read<value_t::object>(j, "Watermark Pos", o.watermarkPos);
    read_number(j, "Watermark Scale", o.watermarkScale);
}

static void from_json(const json& j, Plots& o)
{
    read(j, "Enabled", o.enabled);
    read_number(j, "FPS Mode", o.FPS);
    read_number(j, "FPS Refrash Rate", o.FPSRefresh);
    read_number(j, "FPS Info mode", o.FPSInfo);
    read(j, "FPS Style switch", o.FPSStyle);
    read<value_t::object>(j, "FPS Style Lines", o.FPSStyleLines);
    read<value_t::object>(j, "FPS Style Lines Hovered", o.FPSStyleLinesHovered);
    read<value_t::object>(j, "FPS Style Histogram", o.FPSStyleHistogram);
    read<value_t::object>(j, "FPS Style Histogram Hovered", o.FPSStyleHistogramHovered);
    read_number(j, "FPS Pos", o.FPSPos);
    read_number(j, "FPS Size", o.FPSSize);
    read_number(j, "FPS Scale", o.FPSScale);
    read_number(j, "Ping Mode", o.ping);
    read_number(j, "Ping Refrash Rate", o.pingRefresh);
    read_number(j, "Ping Info mode", o.pingInfo);
    read(j, "Ping Style switch", o.pingStyle);
    read<value_t::object>(j, "Ping Style Lines", o.pingStyleLines);
    read<value_t::object>(j, "Ping Style Lines Hovered", o.pingStyleLinesHovered);
    read<value_t::object>(j, "Ping Style Histogram", o.pingStyleHistogram);
    read<value_t::object>(j, "Ping Style Histogram Hovered", o.pingStyleHistogramHovered);
    read_number(j, "Ping Pos", o.pingPos);
    read_number(j, "Ping Size", o.pingSize);
    read_number(j, "Ping Scale", o.pingScale);
    read_number(j, "Velocity Mode", o.velocity);
    read_number(j, "Velocity Refrash Rate", o.velocityRefresh);
    read_number(j, "Velocity Info mode", o.velocityInfo);
    read(j, "Velocity Style switch", o.velocityStyle);
    read<value_t::object>(j, "Velocity Style Lines", o.velocityStyleLines);
    read<value_t::object>(j, "Velocity Style Lines Hovered", o.velocityStyleLinesHovered);
    read<value_t::object>(j, "Velocity Style Histogram", o.velocityStyleHistogram);
    read<value_t::object>(j, "Velocity Style Histogram Hovered", o.velocityStyleHistogramHovered);
    read_number(j, "Velocity Pos", o.velocityPos);
    read_number(j, "Velocity Size", o.velocitySize);
    read_number(j, "Velocity Scale", o.velocityScale);
}

static void from_json(const json& j, HitMarker& o)
{
    read<value_t::object>(j, "HitMarker", o.hitMarker);
    read_number(j, "HitMarker Length", o.hitMarkerLength);
    read_number(j, "HitMarker Time", o.hitMarkerTime);
    read<value_t::object>(j, "Hit marker damage indicator", o.hitMarkerDamageIndicator);
    read(j, "Hit marker damage indicator Customize", o.hitMarkerDamageIndicatorCustomize);
    read_number(j, "Hit marker damage indicator Dist", o.hitMarkerDamageIndicatorDist);
    read_number(j, "Hit marker damage indicator Ratio", o.hitMarkerDamageIndicatorRatio);
}

#ifdef _WIN32
static void from_json(const json& j, Radio& o)
{
    read_number(j, "Radio Station", o.station);
    read_number(j, "Radio Volume", o.volume);
    read(j, "Radio Mute", o.mute);
}
#endif

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

    read<value_t::object>(j, "Rainbow Bar", miscConfig.rainbowBar);
    read<value_t::object>(j, "Watermark", miscConfig.watermark);
    read<value_t::object>(j, "Plots", miscConfig.plots);
    read<value_t::object>(j, "HitMarker", miscConfig.hitMarker);
#ifdef _WIN32
    read<value_t::object>(j, "Radio", miscConfig.radio);
#endif

    read_number(j, "Menu Color", miscConfig.menuColors);
    if (j.contains("Colors") && j["Colors"].is_object()) {
        const auto& colors = j["Colors"];
        ImGuiStyle& style = ImGui::GetStyle();
        for (int i = 0; i < ImGuiCol_COUNT; i++)
            style.Colors[i] = colors[ImGui::GetStyleColorName(i)];
    }
    read<value_t::object>(j, "Menu color Custom (Easy)", miscConfig.customEasy);

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
