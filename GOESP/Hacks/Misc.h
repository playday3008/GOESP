#pragma once

#include "../ConfigStructs.h"

class GameEvent;
struct ImDrawList;

namespace Misc
{
    void drawPreESP(ImDrawList* drawList) noexcept;
    void drawPostESP(ImDrawList* drawList) noexcept;

    void updateEventListeners(bool forceRemove = false) noexcept;
    void purchaseList(GameEvent* event = nullptr) noexcept;
    void hitEffect(GameEvent& event) noexcept;

    void drawGUI() noexcept;
    bool ignoresFlashbang() noexcept;

    void panicKey() noexcept;
    void hitMarker(GameEvent* event = nullptr) noexcept;
    void hitMarkerDamageIndicator(ImDrawList* drawList = nullptr, GameEvent* event = nullptr) noexcept;

    json toJSON() noexcept;
    void fromJSON(const json& j) noexcept;
}
