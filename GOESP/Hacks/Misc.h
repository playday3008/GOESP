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
    void draw(ImDrawList* drawList) noexcept;
    void drawGUI() noexcept;
    bool ignoresFlashbang() noexcept;
    void drawPlayerList() noexcept;
    json toJSON() noexcept;
    void fromJSON(const json& j) noexcept;
}
