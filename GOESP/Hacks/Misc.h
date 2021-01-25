#pragma once

#include "../ConfigStructs.h"

class GameEvent;
struct ImDrawList;

namespace Misc
{
    void drawReloadProgress(ImDrawList* drawList) noexcept;
    void drawRecoilCrosshair(ImDrawList* drawList) noexcept;
    void purchaseList(GameEvent* event = nullptr) noexcept;
    void drawObserverList() noexcept;
    void drawNoscopeCrosshair(ImDrawList* drawList) noexcept;
    void drawFpsCounter() noexcept;
    void drawOffscreenEnemies(ImDrawList* drawList) noexcept;
    void rainbowBar(ImDrawList* drawList) noexcept;
    void drawBombTimer() noexcept;
    void hitMarker(GameEvent* event = nullptr) noexcept;
    void hitMarkerSetDamageIndicator(GameEvent* = nullptr) noexcept;
    void hitMarkerDamageIndicator() noexcept;
    void watermark() noexcept;
    void plots() noexcept;
#ifdef _WIN32
    void updateRadio(bool off = false) noexcept;
    void radio() noexcept;
#endif
    void draw(ImDrawList* drawList) noexcept;
    void drawGUI() noexcept;
    bool ignoresFlashbang() noexcept;
    void drawPlayerList() noexcept;
    void drawMolotovRadii(ImDrawList* drawList) noexcept;

    json toJSON() noexcept;
    void fromJSON(const json& j) noexcept;
}
