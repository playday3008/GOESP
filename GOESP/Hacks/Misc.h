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

    void drawGUI() noexcept;
    bool ignoresFlashbang() noexcept;

    void panicKey() noexcept;

    json toJSON() noexcept;
    void fromJSON(const json& j) noexcept;
}
