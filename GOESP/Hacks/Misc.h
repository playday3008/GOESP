#pragma once

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
    void watermark() noexcept;
    void hitMarker(GameEvent* event = nullptr) noexcept;
    void hitMarkerSetDamageIndicator(GameEvent* = nullptr) noexcept;
    void hitMarkerDamageIndicator() noexcept;
}
