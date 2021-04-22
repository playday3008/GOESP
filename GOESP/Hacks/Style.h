#pragma once

#include "../ConfigStructs.h"

namespace Style
{
	void drawGUI();
	json toJSON() noexcept;
	void fromJSON(const json& j) noexcept;
}