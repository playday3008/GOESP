#include "Style.h"

#include "../Helpers.h"

#include "../nlohmann/json.hpp"
#include <variant>

struct {
    std::string colors = "Classic";
    std::array<bool, 35> applyStyle{ false };
    std::array<bool, 35> applyLerp{ false };
    std::string fonts = "Noto Sans Regular, 16px";
	
    ImGuiColorEditFlags alpha_flags = ImGuiColorEditFlags_None;

    ImVec4 backgroundColor{ 0.f, 0.f, 0.f ,1.f };
    ImVec4 textColor{ 0.9f, 0.9f, 0.9f ,1.f };
    ImVec4 mainColor{ 0.35f, 0.4f, 0.61f, 1.f };
    ImVec4 mainAccentColor{ 0.4f, 0.4f, 0.9f, 1.f };
    ImVec4 highlightColor{ 0.9f, 0.9f, 0.9f, 1.f };
} styleConfig;

static int colors = 0; // Current color index

static void NodeFont(ImFont* font)
{
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    const bool font_details_opened = ImGui::TreeNode(font, "Font: \"%s\"\n%.2f px, %d glyphs, %d file(s)",
        font->ConfigData[0].Name, font->FontSize, font->Glyphs.Size, font->ConfigDataCount);
    ImGui::SameLine();
	if (ImGui::SmallButton("Set as default"))
	{
		io.FontDefault = font;
        styleConfig.fonts = font->GetDebugName();
	}
    if (!font_details_opened)
        return;

    ImGui::PushFont(font);
    ImGui::Text("The quick brown fox jumps over the lazy dog");
    ImGui::PopFont();
    ImGui::DragFloat("Font scale", &font->Scale, 0.005f, 0.3f, 2.f, "%.1f");   // Scale only this font
    ImGui::SameLine();
    Helpers::HelpMarker(
        "Note than the default embedded font is NOT meant to be scaled.\n\n"
        "Font are currently rendered into bitmaps at a given size at the time of building the atlas. "
        "You may oversample them to get some flexibility with scaling. "
        "You can also render at multiple sizes and select which one to use at runtime.\n\n"
        "(Glimmer of hope: the atlas system will be rewritten in the future to make scaling more flexible.)");
    ImGui::Text("Ascent: %f, Descent: %f, Height: %f", font->Ascent, font->Descent, font->Ascent - font->Descent);
    ImGui::Text("Fallback character: '%c' (U+%04X)", font->FallbackChar, font->FallbackChar);
    ImGui::Text("Ellipsis character: '%c' (U+%04X)", font->EllipsisChar, font->EllipsisChar);
    const int surface_sqrt = static_cast<int>(sqrtf(static_cast<float>(font->MetricsTotalSurface)));
    ImGui::Text("Texture Area: about %d px ~%dx%d px", font->MetricsTotalSurface, surface_sqrt, surface_sqrt);
    for (int config_i = 0; config_i < font->ConfigDataCount; config_i++)
        if (font->ConfigData)
            if (const ImFontConfig* cfg = &font->ConfigData[config_i])
                ImGui::BulletText("Input %d: \'%s\', Oversample: (%d,%d), PixelSnapH: %d, Offset: (%.1f,%.1f)",
                    config_i, cfg->Name, cfg->OversampleH, cfg->OversampleV, cfg->PixelSnapH, cfg->GlyphOffset.x, cfg->GlyphOffset.y);
    if (ImGui::TreeNode("Glyphs", "Glyphs (%d)", font->Glyphs.Size))
    {
        // Display all glyphs of the fonts in separate pages of 256 characters
        const ImU32 glyph_col = ImGui::GetColorU32(ImGuiCol_Text);
        for (unsigned int base = 0; base <= IM_UNICODE_CODEPOINT_MAX; base += 256)
        {
            // Skip ahead if a large bunch of glyphs are not present in the font (test in chunks of 4k)
            // This is only a small optimization to reduce the number of iterations when IM_UNICODE_MAX_CODEPOINT
            // is large // (if ImWchar==ImWchar32 we will do at least about 272 queries here)
            if (!(base & 4095) && font->IsGlyphRangeUnused(base, base + 4095))
            {
                base += 4096 - 256;
                continue;
            }

            int count = 0;
            for (unsigned int n = 0; n < 256; n++)
                if (font->FindGlyphNoFallback(static_cast<ImWchar>(base + n)))
                    count++;
            if (count <= 0)
                continue;
            if (!ImGui::TreeNode(reinterpret_cast<void*>(static_cast<intptr_t>(base)), "U+%04X..U+%04X (%d %s)", base, base + 255, count, count > 1 ? "glyphs" : "glyph"))
                continue;
            const float cell_size = font->FontSize * 1;
            const float cell_spacing = style.ItemSpacing.y;
            const ImVec2 base_pos = ImGui::GetCursorScreenPos();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            for (unsigned int n = 0; n < 256; n++)
            {
                // We use ImFont::RenderChar as a shortcut because we don't have UTF-8 conversion functions
                // available here and thus cannot easily generate a zero-terminated UTF-8 encoded string.
                ImVec2 cell_p1(base_pos.x + static_cast<float>(n % 16u) * (cell_size + cell_spacing), base_pos.y + static_cast<float>(n) / 16.f * (cell_size + cell_spacing));
                ImVec2 cell_p2(cell_p1.x + cell_size, cell_p1.y + cell_size);
                const ImFontGlyph* glyph = font->FindGlyphNoFallback(static_cast<ImWchar>(base + n));
                draw_list->AddRect(cell_p1, cell_p2, glyph ? IM_COL32(255, 255, 255, 100) : IM_COL32(255, 255, 255, 50));
                if (glyph)
                    font->RenderChar(draw_list, cell_size, cell_p1, glyph_col, static_cast<ImWchar>(base + n));
                if (glyph && ImGui::IsMouseHoveringRect(cell_p1, cell_p2))
                {
                    ImGui::BeginTooltip();
                    ImGui::Text("Codepoint: U+%04X", base + n);
                    ImGui::Separator();
                    ImGui::Text("Visible: %d", static_cast<int>(glyph->Visible));
                    ImGui::Text("AdvanceX: %.1f", glyph->AdvanceX);
                    ImGui::Text("Pos: (%.2f,%.2f)->(%.2f,%.2f)", glyph->X0, glyph->Y0, glyph->X1, glyph->Y1);
                    ImGui::Text("UV: (%.3f,%.3f)->(%.3f,%.3f)", glyph->U0, glyph->V0, glyph->U1, glyph->V1);
                    ImGui::EndTooltip();
                }
            }
            ImGui::Dummy(ImVec2((cell_size + cell_spacing) * 16, (cell_size + cell_spacing) * 16));
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
    ImGui::TreePop();
}

static void CustomEasy()
{
    constexpr auto GetColor     = [](ImVec4 color, const float a = 0.8f) { color.w *= a; return color; };
    constexpr auto Darken       = [](ImVec4 color, const float a)
    {
        color.x = std::max(0.f, color.x - 1.f * a); // if color.x > a, output will be 0
        color.y = std::max(0.f, color.y - 1.f * a); // if color.y > a, output will be 0
        color.z = std::max(0.f, color.z - 1.f * a); // if color.z > a, output will be 0
        return color;
    };
    constexpr auto Lighten      = [](ImVec4 color, const float a)
    {
        color.x = std::min(1.f, color.x + 1.f * a); // if color.x + a > 1, output will be 1
        color.y = std::min(1.f, color.y + 1.f * a); // if color.y + a > 1, output will be 1
        color.z = std::min(1.f, color.z + 1.f * a); // if color.z + a > 1, output will be 1
        return color;
    };
    constexpr auto Disabled     = [=](const ImVec4 color) { return Darken(color, 0.6f); };
    constexpr auto Hovered      = [=](const ImVec4 color) { return Lighten(color, 0.2f); };
    constexpr auto Active       = [=](const ImVec4 color) { return Lighten({ color.x ,color.y, color.z, 1.f }, 0.1f); };
    constexpr auto Collapsed    = [=](const ImVec4 color) { return Darken(color, 0.2f); };

    auto& style = ImGui::GetStyle();
	
    style.Colors[ImGuiCol_Text]                     = GetColor(styleConfig.textColor);
    style.Colors[ImGuiCol_TextDisabled]             = Disabled(style.Colors[ImGuiCol_Text]);
    style.Colors[ImGuiCol_WindowBg]                 = GetColor(styleConfig.backgroundColor);
    style.Colors[ImGuiCol_ChildBg]                  = { 0.f, 0.f, 0.f, 0.2f };
    style.Colors[ImGuiCol_PopupBg]                  = GetColor(styleConfig.backgroundColor, 0.9f);
    style.Colors[ImGuiCol_Border]                   = Lighten(style.Colors[ImGuiCol_WindowBg], 0.4f);
    style.Colors[ImGuiCol_BorderShadow]             = { 0.f, 0.f, 0.f, 0.8f };
    style.Colors[ImGuiCol_FrameBg]                  = GetColor(styleConfig.mainAccentColor, 0.4f);
    style.Colors[ImGuiCol_FrameBgHovered]           = Hovered(style.Colors[ImGuiCol_FrameBg]);
    style.Colors[ImGuiCol_FrameBgActive]            = Active(style.Colors[ImGuiCol_FrameBg]);
    style.Colors[ImGuiCol_TitleBg]                  = style.Colors[ImGuiCol_WindowBg];
    style.Colors[ImGuiCol_TitleBgActive]            = Active(style.Colors[ImGuiCol_TitleBg]);
    style.Colors[ImGuiCol_TitleBgCollapsed]         = Collapsed(style.Colors[ImGuiCol_TitleBg]);
    style.Colors[ImGuiCol_MenuBarBg]                = Collapsed(style.Colors[ImGuiCol_WindowBg]);
    style.Colors[ImGuiCol_ScrollbarBg]              = Lighten(GetColor(styleConfig.backgroundColor, 0.5f), 0.4f);
    style.Colors[ImGuiCol_ScrollbarGrab]            = Lighten(style.Colors[ImGuiCol_WindowBg], 0.3f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]     = Hovered(style.Colors[ImGuiCol_ScrollbarGrab]);
    style.Colors[ImGuiCol_ScrollbarGrabActive]      = Active(style.Colors[ImGuiCol_ScrollbarGrab]);
    style.Colors[ImGuiCol_CheckMark]                = GetColor(styleConfig.highlightColor);
    style.Colors[ImGuiCol_SliderGrab]               = style.Colors[ImGuiCol_CheckMark];
    style.Colors[ImGuiCol_SliderGrabActive]         = Active(style.Colors[ImGuiCol_SliderGrab]);
    style.Colors[ImGuiCol_Button]                   = GetColor(styleConfig.mainColor);
    style.Colors[ImGuiCol_ButtonHovered]            = Hovered(style.Colors[ImGuiCol_Button]);
    style.Colors[ImGuiCol_ButtonActive]             = Active(style.Colors[ImGuiCol_Button]);
    style.Colors[ImGuiCol_Header]                   = GetColor(styleConfig.mainAccentColor);
    style.Colors[ImGuiCol_HeaderHovered]            = Hovered(style.Colors[ImGuiCol_Header]);
    style.Colors[ImGuiCol_HeaderActive]             = Active(style.Colors[ImGuiCol_Header]);
    style.Colors[ImGuiCol_Separator]                = style.Colors[ImGuiCol_Border];
    style.Colors[ImGuiCol_SeparatorHovered]         = Hovered(style.Colors[ImGuiCol_Separator]);
    style.Colors[ImGuiCol_SeparatorActive]          = Active(style.Colors[ImGuiCol_Separator]);
    style.Colors[ImGuiCol_ResizeGrip]               = GetColor(styleConfig.mainColor, 0.2f);
    style.Colors[ImGuiCol_ResizeGripHovered]        = Hovered(style.Colors[ImGuiCol_ResizeGrip]);
    style.Colors[ImGuiCol_ResizeGripActive]         = Active(style.Colors[ImGuiCol_ResizeGrip]);
    style.Colors[ImGuiCol_Tab]                      = GetColor(styleConfig.mainColor, 0.6f);
    style.Colors[ImGuiCol_TabHovered]               = Hovered(style.Colors[ImGuiCol_Tab]);
    style.Colors[ImGuiCol_TabActive]                = Active(style.Colors[ImGuiCol_Tab]);
    style.Colors[ImGuiCol_TabUnfocused]             = style.Colors[ImGuiCol_Tab];
    style.Colors[ImGuiCol_TabUnfocusedActive]       = style.Colors[ImGuiCol_TabActive];
    style.Colors[ImGuiCol_PlotLines]                = style.Colors[ImGuiCol_CheckMark];
    style.Colors[ImGuiCol_PlotLinesHovered]         = Hovered(style.Colors[ImGuiCol_PlotLines]);
    style.Colors[ImGuiCol_PlotHistogram]            = style.Colors[ImGuiCol_CheckMark];
    style.Colors[ImGuiCol_PlotHistogramHovered]     = Hovered(style.Colors[ImGuiCol_PlotHistogram]);
    //style.Colors[ImGuiCol_TableHeaderBg]            = 
    //style.Colors[ImGuiCol_TableBorderStrong]        = 
    //style.Colors[ImGuiCol_TableBorderLight]         = 
    style.Colors[ImGuiCol_TableRowBg]               = { 0.f, 0.f, 0.f, 0.f };
    //style.Colors[ImGuiCol_TableRowBgAlt]            = 
    style.Colors[ImGuiCol_TextSelectedBg]           = GetColor(styleConfig.highlightColor, 0.4f);
    style.Colors[ImGuiCol_DragDropTarget]           = style.Colors[ImGuiCol_CheckMark];
    style.Colors[ImGuiCol_NavHighlight]             = { 1.f, 1.f, 1.f, 0.8f };
    style.Colors[ImGuiCol_NavWindowingHighlight]    = style.Colors[ImGuiCol_NavHighlight];
    style.Colors[ImGuiCol_NavWindowingDimBg]        = { 1.f, 1.f, 1.f, 0.2f };
    style.Colors[ImGuiCol_ModalWindowDimBg]         = { 0.f, 0.f, 0.f, 0.6f };
}

static std::array avaliableColors{
    "Classic",
    "Dark",
    "Light",
    "Custom (Easy)",
    "Fatality (WIP)",
    "OneTap-V3 (WIP)",
    "Mutiny (WIP)",
    "CSGOSimple (WIP)",
    "CSGOSimple(Extender) (WIP)",
    "BoyNextHook (WIP)",
    "NIXWARE (WIP)",
    "Sensum (Light) (WIP)",
    "Sensum (Dark) (WIP)",
    "Setupim (WIP)",
    "Monochrome (WIP)",
    "Procedural (WIP)",
    "Pagghiu (WIP)",
    "Doug (WIP)",
    "Microsoft (WIP)",
    "Darcula (WIP)",
    "UE4 (WIP)",
    "Cherry (WIP)",
    "LightGreen (WIP)",
    "Photoshop (WIP)",
    "CorporateGrey (WIP)",
    "Violet (WIP)",
    "Raikiri (WIP)",
    "VGUI (WIP)",
    "Gold & Black (WIP)",
    "Sewer (WIP)",
    "VS (WIP)",
    "OverShifted (WIP)",
    "RayTeak (WIP)",
    "Aiekick (Green/Blue)",
    "Aiekick (Red/Dark)"
};

static std::array avaliableColorsStyle{
    false, // Classic
    false, // Dark
    false, // Light
    false, // Custom (Easy)
    false, // Fatality (WIP)
    false, // OneTap-V3 (WIP)
    true,  // Mutiny (WIP)
    true,  // CSGOSimple (WIP)
    true,  // CSGOSimple(Extender) (WIP)
    true,  // BoyNextHook (WIP)
    true,  // NIXWARE (WIP)
    true,  // Sensum (Light) (WIP)
    true,  // Sensum (Dark) (WIP)
    true,  // Setupim (WIP)
    true,  // Monochrome (WIP)
    false, // Procedural (WIP)
    true,  // Pagghiu (WIP)
    false, // Doug (WIP)
    true,  // Microsoft (WIP)
    true,  // Darcula (WIP)
    false, // UE4 (WIP)
    true,  // Cherry (WIP)
    true,  // LightGreen (WIP)
    true,  // Photoshop (WIP)
    true,  // CorporateGrey (WIP)
    true,  // Violet (WIP)
    true,  // Raikiri (WIP)
    true,  // VGUI (WIP)
    true,  // Gold & Black (WIP)
    true,  // Sewer (WIP)
    true,  // VS (WIP)
    true,  // OverShifted (WIP)
    true,  // RayTeak (WIP)
    false, // Aiekick (Green/Blue)
    false, // Aiekick (Red/Dark)
};

static std::array avaliableColorsLerp{
    false, // Classic
    false, // Dark
    false, // Light
    false, // Custom (Easy)
    false, // Fatality (WIP)
    false, // OneTap-V3 (WIP)
    true,  // Mutiny (WIP)
    false, // CSGOSimple (WIP)
    true,  // CSGOSimple(Extender) (WIP)
    true,  // BoyNextHook (WIP)
    true,  // NIXWARE (WIP)
    true,  // Sensum (Light) (WIP)
    true,  // Sensum (Dark) (WIP)
    true,  // Setupim (WIP)
    true,  // Monochrome (WIP)
    true,  // Procedural (WIP)
    false, // Pagghiu (WIP)
    true,  // Doug (WIP)
    false, // Microsoft (WIP)
    true,  // Darcula (WIP)
    true,  // UE4 (WIP)
    true,  // Cherry (WIP)
    true,  // LightGreen (WIP)
    false, // Photoshop (WIP)
    false, // CorporateGrey (WIP)
    false, // Violet (WIP)
    false, // Raikiri (WIP)
    false, // VGUI (WIP)
    false, // Gold & Black (WIP)
    false, // Sewer (WIP)
    false, // VS (WIP)
    false, // OverShifted (WIP)
    true,  // RayTeak (WIP)
    false, // Aiekick (Green/Blue)
    false, // Aiekick (Red/Dark)
};

static void UpdateColors(int colorId, bool applyStyle = false, bool applyLerp = false)
{
    auto& style = ImGui::GetStyle();
	
    switch (colorId)
    {
    case 0: ImGui::StyleColorsClassic(); break;
    case 1: ImGui::StyleColorsDark(); break;
    case 2: ImGui::StyleColorsLight(); break;
    case 3: CustomEasy(); break;
#pragma region Fatality (WIP)
    case 4:
        style.Colors[ImGuiCol_Text]                     = ImColor(177, 170, 212, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(0, 0, 0, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(33, 26, 68, 255);
        style.Colors[ImGuiCol_ChildBg]                  = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(25, 18, 62, 240);
        style.Colors[ImGuiCol_Border]                   = ImColor(58, 53, 85, 255);
        style.Colors[ImGuiCol_BorderShadow]             = ImColor(31, 31, 31, 0);
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(26, 19, 63, 255);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(225, 14, 95, 102);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(177, 0, 67, 255);
        style.Colors[ImGuiCol_TitleBg]                  = ImColor(22, 19, 46, 255);
        style.Colors[ImGuiCol_TitleBgActive]            = ImColor(33, 26, 68, 255);
        style.Colors[ImGuiCol_TitleBgCollapsed]         = ImColor(162, 10, 116, 130);
        style.Colors[ImGuiCol_MenuBarBg]                = ImColor(36, 36, 36, 255);
        style.Colors[ImGuiCol_ScrollbarBg]              = ImColor(25, 18, 51, 135);
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(189, 9, 83, 255);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(211, 0, 84, 255);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = ImColor(191, 18, 88, 255);
        style.Colors[ImGuiCol_CheckMark]                = ImColor(184, 8, 83, 255);
        style.Colors[ImGuiCol_SliderGrab]               = ImColor(191, 18, 88, 255);
        style.Colors[ImGuiCol_SliderGrabActive]         = ImColor(174, 10, 83, 255);
        style.Colors[ImGuiCol_Button]                   = ImColor(223, 5, 91, 209);
        style.Colors[ImGuiCol_ButtonHovered]            = ImColor(208, 4, 82, 255);
        style.Colors[ImGuiCol_ButtonActive]             = ImColor(191, 18, 88, 255);
        style.Colors[ImGuiCol_Header]                   = ImColor(174, 10, 83, 79);
        style.Colors[ImGuiCol_HeaderHovered]            = ImColor(233, 0, 104, 255);
        style.Colors[ImGuiCol_HeaderActive]             = ImColor(236, 9, 80, 231);
        style.Colors[ImGuiCol_Separator]                = ImColor(42, 42, 44, 128);
        style.Colors[ImGuiCol_SeparatorHovered]         = ImColor(243, 13, 157, 199);
        style.Colors[ImGuiCol_SeparatorActive]          = ImColor(255, 0, 99, 255);
        style.Colors[ImGuiCol_ResizeGrip]               = ImColor(196, 207, 221, 0);
        style.Colors[ImGuiCol_ResizeGripHovered]        = ImColor(191, 18, 88, 171);
        style.Colors[ImGuiCol_ResizeGripActive]         = ImColor(191, 18, 88, 242);
        style.Colors[ImGuiCol_Tab]                      = ImColor(37, 0, 53, 0);
        style.Colors[ImGuiCol_TabHovered]               = ImColor(174, 10, 83, 231);
        style.Colors[ImGuiCol_TabActive]                = ImColor(235, 10, 78, 224);
        style.Colors[ImGuiCol_TabUnfocused]             = ImColor(31, 31, 31, 248);
        style.Colors[ImGuiCol_TabUnfocusedActive]       = ImColor(191, 18, 88, 255);
        style.Colors[ImGuiCol_PlotLines]                = ImColor(156, 156, 156, 255);
        style.Colors[ImGuiCol_PlotLinesHovered]         = ImColor(191, 18, 88, 255);
        style.Colors[ImGuiCol_PlotHistogram]            = style.Colors[ImGuiCol_PlotLines];
        style.Colors[ImGuiCol_PlotHistogramHovered]     = style.Colors[ImGuiCol_PlotLinesHovered];
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(191, 18, 88, 89);
        style.Colors[ImGuiCol_DragDropTarget]           = ImColor(191, 18, 88, 230);
        style.Colors[ImGuiCol_NavHighlight]             = ImColor(191, 18, 88, 255);
        style.Colors[ImGuiCol_NavWindowingHighlight]    = ImColor(254, 0, 97, 178);
        style.Colors[ImGuiCol_NavWindowingDimBg]        = ImColor(223, 26, 108, 255);
        style.Colors[ImGuiCol_ModalWindowDimBg]         = ImColor(255, 0, 159, 89);
        break;
#pragma endregion
#pragma region OneTap-V3 (WIP)
    case 5:
        style.Colors[ImGuiCol_Text]                     = ImColor(219, 222, 231, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(128, 128, 128, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(43, 46, 55, 255);
        style.Colors[ImGuiCol_ChildBg]                  = ImColor(111, 111, 111, 0);
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(20, 20, 20, 240);
        style.Colors[ImGuiCol_Border]                   = ImColor(110, 110, 128, 128);
        style.Colors[ImGuiCol_BorderShadow]             = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(28, 31, 38, 138);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(221, 221, 221, 102);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(79, 84, 89, 171);
        style.Colors[ImGuiCol_TitleBg]                  = ImColor(239, 153, 50, 255);
        style.Colors[ImGuiCol_TitleBgActive]            = style.Colors[ImGuiCol_TitleBg];
        style.Colors[ImGuiCol_TitleBgCollapsed]         = ImColor(0, 0, 0, 130);
        style.Colors[ImGuiCol_MenuBarBg]                = ImColor(37, 37, 37, 255);
        style.Colors[ImGuiCol_ScrollbarBg]              = ImColor(5, 5, 5, 135);
        style.Colors[ImGuiCol_ScrollbarGrab]            = style.Colors[ImGuiCol_TitleBg];
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(105, 105, 105, 255);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = ImColor(130, 130, 130, 255);
        style.Colors[ImGuiCol_CheckMark]                = style.Colors[ImGuiCol_TitleBg];
        style.Colors[ImGuiCol_SliderGrab]               = style.Colors[ImGuiCol_TitleBg];
        style.Colors[ImGuiCol_SliderGrabActive]         = style.Colors[ImGuiCol_TitleBg];
        style.Colors[ImGuiCol_Button]                   = ImColor(28, 31, 38, 144);
        style.Colors[ImGuiCol_ButtonHovered]            = style.Colors[ImGuiCol_TitleBg];
        style.Colors[ImGuiCol_ButtonActive]             = ImColor(161, 168, 174, 255);
        style.Colors[ImGuiCol_Header]                   = ImColor(206, 206, 206, 79);
        style.Colors[ImGuiCol_HeaderHovered]            = ImColor(239, 153, 50, 148);
        style.Colors[ImGuiCol_HeaderActive]             = style.Colors[ImGuiCol_TitleBg];
        style.Colors[ImGuiCol_Separator]                = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_SeparatorHovered]         = ImColor(152, 153, 155, 199);
        style.Colors[ImGuiCol_SeparatorActive]          = ImColor(160, 173, 189, 255);
        style.Colors[ImGuiCol_ResizeGrip]               = ImColor(18, 18, 17, 0);
        style.Colors[ImGuiCol_ResizeGripHovered]        = ImColor(212, 216, 221, 171);
        style.Colors[ImGuiCol_ResizeGripActive]         = ImColor(239, 153, 50, 242);
        style.Colors[ImGuiCol_Tab]                      = ImColor(13, 13, 13, 0);
        style.Colors[ImGuiCol_TabHovered]               = ImColor(128, 128, 128, 204);
        style.Colors[ImGuiCol_TabActive]                = ImColor(32, 35, 45, 255);
        style.Colors[ImGuiCol_TabUnfocused]             = ImColor(17, 26, 38, 248);
        style.Colors[ImGuiCol_TabUnfocusedActive]       = ImColor(221, 221, 221, 255);
        style.Colors[ImGuiCol_PlotLines]                = ImColor(156, 156, 156, 255);
        style.Colors[ImGuiCol_PlotLinesHovered]         = ImColor(82, 82, 82, 255);
        style.Colors[ImGuiCol_PlotHistogram]            = style.Colors[ImGuiCol_TitleBg];
        style.Colors[ImGuiCol_PlotHistogramHovered]     = style.Colors[ImGuiCol_TitleBg];
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(231, 231, 231, 89);
        style.Colors[ImGuiCol_DragDropTarget]           = ImColor(239, 153, 50, 230);
        style.Colors[ImGuiCol_NavHighlight]             = ImColor(59, 59, 59, 255);
        style.Colors[ImGuiCol_NavWindowingHighlight]    = ImColor(73, 73, 73, 178);
        style.Colors[ImGuiCol_NavWindowingDimBg]        = ImColor(204, 204, 204, 51);
        style.Colors[ImGuiCol_ModalWindowDimBg]         = ImColor(204, 204, 204, 89);
        break;
#pragma endregion
#pragma region Mutiny (WIP)
    case 6:
        if (applyStyle)
        {
            style.WindowTitleAlign  = ImVec2(0.5f, 0.5f);
            style.WindowPadding     = ImVec2(15.f, 15.f);
            style.WindowRounding    = 5.f;
            style.FramePadding      = ImVec2(5.f, 5.f);
            style.FrameRounding     = 4.f;
            style.ItemSpacing       = ImVec2(12.f, 8.f);
            style.ItemInnerSpacing  = ImVec2(8.f, 6.f);
            style.IndentSpacing     = 25.f;
            style.ScrollbarSize     = 15.f;
            style.ScrollbarRounding = 9.f;
            style.GrabMinSize       = 5.f;
            style.GrabRounding      = 3.f;
        }

        style.Colors[ImGuiCol_Text]                     = ImColor(204, 204, 212, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(61, 59, 74, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(15, 13, 18, 255);
        style.Colors[ImGuiCol_ChildBg]                  = ImColor(18, 18, 23, 255);
        style.Colors[ImGuiCol_PopupBg]                  = style.Colors[ImGuiCol_ChildBg];
        style.Colors[ImGuiCol_Border]                   = ImColor(204, 204, 212, 224);
        style.Colors[ImGuiCol_BorderShadow]             = ImColor(235, 232, 224, 0);
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(26, 23, 31, 255);
        style.Colors[ImGuiCol_FrameBgHovered]           = style.Colors[ImGuiCol_TextDisabled];
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(143, 143, 148, 255);
        style.Colors[ImGuiCol_TitleBg]                  = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_TitleBgActive]            = ImColor(255, 250, 242, 191);
        style.Colors[ImGuiCol_TitleBgCollapsed]         = style.Colors[ImGuiCol_ChildBg];
        style.Colors[ImGuiCol_MenuBarBg]                = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ScrollbarBg]              = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(204, 204, 212, 79);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_CheckMark]                = style.Colors[ImGuiCol_ScrollbarGrab];
        style.Colors[ImGuiCol_SliderGrab]               = style.Colors[ImGuiCol_ScrollbarGrab];
        style.Colors[ImGuiCol_SliderGrabActive]         = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_Button]                   = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ButtonHovered]            = style.Colors[ImGuiCol_TextDisabled];
        style.Colors[ImGuiCol_ButtonActive]             = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Header]                   = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_HeaderHovered]            = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_HeaderActive]             = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_Separator]                = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_SeparatorHovered]         = style.Colors[ImGuiCol_TextDisabled];
        style.Colors[ImGuiCol_SeparatorActive]          = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_ResizeGrip]               = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_ResizeGripHovered]        = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_ResizeGripActive]         = style.Colors[ImGuiCol_WindowBg];
        if (applyLerp)
        {
            style.Colors[ImGuiCol_Tab]                      = ImLerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 0.8f);
            style.Colors[ImGuiCol_TabHovered]               = style.Colors[ImGuiCol_HeaderHovered];
            style.Colors[ImGuiCol_TabActive]                = ImLerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.6f);
            style.Colors[ImGuiCol_TabUnfocused]             = ImLerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.8f);
            style.Colors[ImGuiCol_TabUnfocusedActive]       = ImLerp(style.Colors[ImGuiCol_TabActive], style.Colors[ImGuiCol_TitleBg], 0.4f);
        }
        style.Colors[ImGuiCol_PlotLines]                = ImColor(102, 99, 97, 161);
        style.Colors[ImGuiCol_PlotLinesHovered]         = ImColor(64, 255, 0, 255);
        style.Colors[ImGuiCol_PlotHistogram]            = style.Colors[ImGuiCol_PlotLines];
        style.Colors[ImGuiCol_PlotHistogramHovered]     = style.Colors[ImGuiCol_PlotLinesHovered];
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(64, 255, 0, 110);
        //style.Colors[ImGuiCol_DragDropTarget]           = 
        //style.Colors[ImGuiCol_NavHighlight]             = 
        //style.Colors[ImGuiCol_NavWindowingHighlight]    = 
    	//style.Colors[ImGuiCol_NavWindowingDimBg]        = 
        style.Colors[ImGuiCol_ModalWindowDimBg]         = ImColor(255, 250, 242, 186);
        break;
#pragma endregion
#pragma region CSGOSimple (WIP)
    case 7:
        ImGui::StyleColorsDark(); //CSGOSimple b1g brain solution
        if (applyStyle)
        {
            style.FrameRounding     = 0.f;
            style.WindowRounding    = 0.f;
            style.ChildRounding     = 0.f;
        }

        //style.Colors[ImGuiCol_Text]                     = 
    	//style.Colors[ImGuiCol_TextDisabled]             = 
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(0, 2, 31, 240);
        //style.Colors[ImGuiCol_ChildBg]                  = 
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(19, 36, 53, 255);
        //style.Colors[ImGuiCol_Border]                   = 
        //style.Colors[ImGuiCol_BorderShadow]             = 
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(51, 64, 76, 255);
        //style.Colors[ImGuiCol_FrameBgHovered]           = 
        //style.Colors[ImGuiCol_FrameBgActive]            = 
        //style.Colors[ImGuiCol_TitleBg]                  = 
        //style.Colors[ImGuiCol_TitleBgActive]            = 
        //style.Colors[ImGuiCol_TitleBgCollapsed]         = 
        //style.Colors[ImGuiCol_MenuBarBg]                = 
        //style.Colors[ImGuiCol_ScrollbarBg]              = 
        //style.Colors[ImGuiCol_ScrollbarGrab]            = 
        //style.Colors[ImGuiCol_ScrollbarGrabHovered]     = 
        //style.Colors[ImGuiCol_ScrollbarGrabActive]      = 
        //style.Colors[ImGuiCol_CheckMark]                = 
        //style.Colors[ImGuiCol_SliderGrab]               = 
        //style.Colors[ImGuiCol_SliderGrabActive]         = 
        style.Colors[ImGuiCol_Button]                   = ImColor(66, 150, 250, 171);
        style.Colors[ImGuiCol_ButtonHovered]            = ImColor(0, 139, 255, 255);
        style.Colors[ImGuiCol_ButtonActive]             = ImColor(15, 106, 250, 255);
        style.Colors[ImGuiCol_Header]                   = style.Colors[ImGuiCol_Button];
        style.Colors[ImGuiCol_HeaderHovered]            = ImColor(66, 150, 250, 255);
        //style.Colors[ImGuiCol_HeaderActive]             = 
        //style.Colors[ImGuiCol_Separator]                = style.Colors[ImGuiCol_Border];
        //style.Colors[ImGuiCol_SeparatorHovered]         = 
        //style.Colors[ImGuiCol_SeparatorActive]          = 
        //style.Colors[ImGuiCol_ResizeGrip]               = 
        //style.Colors[ImGuiCol_ResizeGripHovered]        = 
        //style.Colors[ImGuiCol_ResizeGripActive]         = 
        /*if (applyLerp)
        {
            style.Colors[ImGuiCol_Tab]                      = ImLerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 0.8f);
            style.Colors[ImGuiCol_TabHovered]               = style.Colors[ImGuiCol_HeaderHovered];
            style.Colors[ImGuiCol_TabActive]                = ImLerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.6f);
            style.Colors[ImGuiCol_TabUnfocused]             = ImLerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.8f);
            style.Colors[ImGuiCol_TabUnfocusedActive]       = ImLerp(style.Colors[ImGuiCol_TabActive], style.Colors[ImGuiCol_TitleBg], 0.4f);
        }*/
        //style.Colors[ImGuiCol_PlotLines]                = 
        //style.Colors[ImGuiCol_PlotLinesHovered]         = 
        //style.Colors[ImGuiCol_PlotHistogram]            = 
        //style.Colors[ImGuiCol_PlotHistogramHovered]     = 
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        //style.Colors[ImGuiCol_TextSelectedBg]           = 
        //style.Colors[ImGuiCol_DragDropTarget]           = 
        //style.Colors[ImGuiCol_NavHighlight]             = 
        //style.Colors[ImGuiCol_NavWindowingHighlight]    = 
        //style.Colors[ImGuiCol_NavWindowingDimBg]        = 
        //style.Colors[ImGuiCol_ModalWindowDimBg]         = 
    	break;
#pragma endregion
#pragma region CSGOSimple(Extender) (WIP)
    case 8:
        if (applyStyle)
        {
            style.WindowPadding     = ImVec2(13, 13);
            style.WindowRounding    = 0.f;
            style.FramePadding      = ImVec2(5, 5);
            style.FrameRounding     = 0.f;
            style.ItemSpacing       = ImVec2(12, 8);
            style.ItemInnerSpacing  = ImVec2(8, 6);
            style.IndentSpacing     = 25.f;
            style.ScrollbarSize     = 15.f;
            style.ScrollbarRounding = 9.f;
            style.GrabMinSize       = 5.f;
            style.GrabRounding      = 3.f;
        }

        style.Colors[ImGuiCol_Text]                     = ImColor(212, 242, 242, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(99, 204, 204, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(51, 51, 51, 216);
        style.Colors[ImGuiCol_ChildBg]                  = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(13, 13, 26, 230);
        style.Colors[ImGuiCol_Border]                   = ImColor(128, 128, 128, 255);
        style.Colors[ImGuiCol_BorderShadow]             = style.Colors[ImGuiCol_ChildBg];
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(46, 140, 140, 171);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(64, 191, 191, 171);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(41, 125, 125, 115);
        style.Colors[ImGuiCol_TitleBg]                  = ImColor(41, 125, 125, 255);
        style.Colors[ImGuiCol_TitleBgActive]            = ImColor(46, 143, 143, 255);
        style.Colors[ImGuiCol_TitleBgCollapsed]         = ImColor(41, 125, 125, 153);
        style.Colors[ImGuiCol_MenuBarBg]                = ImColor(41, 120, 120, 255);
        style.Colors[ImGuiCol_ScrollbarBg]              = style.Colors[ImGuiCol_ChildBg];
        style.Colors[ImGuiCol_ScrollbarGrab]            = style.Colors[ImGuiCol_ChildBg];
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = style.Colors[ImGuiCol_ChildBg];
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = style.Colors[ImGuiCol_ChildBg];
        style.Colors[ImGuiCol_CheckMark]                = ImColor(112, 207, 207, 255);
        style.Colors[ImGuiCol_SliderGrab]               = ImColor(84, 199, 199, 153);
        style.Colors[ImGuiCol_SliderGrabActive]         = ImColor(84, 199, 199, 255);
        style.Colors[ImGuiCol_Button]                   = ImColor(230, 152, 77, 237);
        style.Colors[ImGuiCol_ButtonHovered]            = ImColor(230, 152, 77, 255);
        style.Colors[ImGuiCol_ButtonActive]             = ImColor(255, 152, 77, 255);
        style.Colors[ImGuiCol_Header]                   = ImColor(38, 112, 112, 255);
        style.Colors[ImGuiCol_HeaderHovered]            = ImColor(84, 199, 199, 171);
        style.Colors[ImGuiCol_HeaderActive]             = style.Colors[ImGuiCol_SliderGrabActive];
        style.Colors[ImGuiCol_Separator]                = ImColor(64, 189, 189, 102);
        style.Colors[ImGuiCol_SeparatorHovered]         = ImColor(64, 189, 189, 153);
        style.Colors[ImGuiCol_SeparatorActive]          = ImColor(64, 189, 189, 255);
        style.Colors[ImGuiCol_ResizeGrip]               = ImColor(51, 156, 156, 255);
        style.Colors[ImGuiCol_ResizeGripHovered]        = ImColor(135, 214, 214, 171);
        style.Colors[ImGuiCol_ResizeGripActive]         = ImColor(135, 214, 214, 255);
    	if (applyLerp)
    	{
            style.Colors[ImGuiCol_Tab]                      = ImLerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 0.8f);
            style.Colors[ImGuiCol_TabHovered]               = style.Colors[ImGuiCol_HeaderHovered];
            style.Colors[ImGuiCol_TabActive]                = ImLerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.6f);
            style.Colors[ImGuiCol_TabUnfocused]             = ImLerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.8f);
            style.Colors[ImGuiCol_TabUnfocusedActive]       = ImLerp(style.Colors[ImGuiCol_TabActive], style.Colors[ImGuiCol_TitleBg], 0.4f);
    	}
        style.Colors[ImGuiCol_PlotLines]                = style.Colors[ImGuiCol_ResizeGripActive];
        style.Colors[ImGuiCol_PlotLinesHovered]         = ImColor(0, 214, 214, 255);
        style.Colors[ImGuiCol_PlotHistogram]            = style.Colors[ImGuiCol_ResizeGripActive];
        style.Colors[ImGuiCol_PlotHistogramHovered]     = style.Colors[ImGuiCol_PlotLinesHovered];
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(33, 102, 102, 255);
        //style.Colors[ImGuiCol_DragDropTarget]           = 
        //style.Colors[ImGuiCol_NavHighlight]             = 
        //style.Colors[ImGuiCol_NavWindowingHighlight]    = 
        //style.Colors[ImGuiCol_NavWindowingDimBg]        = 
        style.Colors[ImGuiCol_ModalWindowDimBg]         = ImColor(23, 69, 69, 171);
    	break;
#pragma endregion
#pragma region BoyNextHook (WIP)
    case 9:
        if (applyStyle)
            style.WindowRounding = 0.f;

        style.Colors[ImGuiCol_Text]                     = ImColor(255, 255, 255, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(128, 128, 128, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(33, 35, 47, 255);
        style.Colors[ImGuiCol_ChildBg]                  = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(20, 20, 20, 240);
        style.Colors[ImGuiCol_Border]                   = ImColor(30, 30, 41, 255);
        style.Colors[ImGuiCol_BorderShadow]             = { 0.f, 0.f, 0.f, 0.f };
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(33, 33, 33, 255);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(38, 38, 38, 255);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(20, 20, 20, 255);
        style.Colors[ImGuiCol_TitleBg]                  = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_TitleBgActive]            = ImColor(35, 35, 35, 255);
        style.Colors[ImGuiCol_TitleBgCollapsed]         = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_MenuBarBg]                = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ScrollbarBg]              = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ScrollbarGrab]            = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(64, 64, 64, 255);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = style.Colors[ImGuiCol_ScrollbarGrabHovered];
        style.Colors[ImGuiCol_CheckMark]                = ImColor(167, 24, 71, 255);
        //style.Colors[ImGuiCol_SliderGrab]               = 
        //style.Colors[ImGuiCol_SliderGrabActive]         = 
        style.Colors[ImGuiCol_Button]                   = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ButtonHovered]            = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_ButtonActive]             = ImColor(135, 135, 135, 255);
        style.Colors[ImGuiCol_Header]                   = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_HeaderHovered]            = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_HeaderActive]             = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_Separator]                = style.Colors[ImGuiCol_Text];
        style.Colors[ImGuiCol_SeparatorHovered]         = style.Colors[ImGuiCol_Text];
        style.Colors[ImGuiCol_SeparatorActive]          = style.Colors[ImGuiCol_Text];
        style.Colors[ImGuiCol_ResizeGrip]               = ImColor(66, 150, 250, 64);
        style.Colors[ImGuiCol_ResizeGripHovered]        = ImColor(66, 150, 250, 171);
        style.Colors[ImGuiCol_ResizeGripActive]         = ImColor(66, 150, 250, 242);
        if (applyLerp)
        {
            style.Colors[ImGuiCol_Tab]                      = ImLerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 0.8f);
            style.Colors[ImGuiCol_TabHovered]               = style.Colors[ImGuiCol_HeaderHovered];
            style.Colors[ImGuiCol_TabActive]                = ImLerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.6f);
            style.Colors[ImGuiCol_TabUnfocused]             = ImLerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.8f);
            style.Colors[ImGuiCol_TabUnfocusedActive]       = ImLerp(style.Colors[ImGuiCol_TabActive], style.Colors[ImGuiCol_TitleBg], 0.4f);
        }
        style.Colors[ImGuiCol_PlotLines]                = ImColor(156, 156, 156, 255);
        style.Colors[ImGuiCol_PlotLinesHovered]         = ImColor(255, 110, 89, 255);
        style.Colors[ImGuiCol_PlotHistogram]            = ImColor(230, 178, 0, 255);
        style.Colors[ImGuiCol_PlotHistogramHovered]     = ImColor(255, 153, 0, 255);
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(66, 150, 250, 89);
        style.Colors[ImGuiCol_DragDropTarget]           = style.Colors[ImGuiCol_FrameBgActive];
        //style.Colors[ImGuiCol_NavHighlight]             = 
        //style.Colors[ImGuiCol_NavWindowingHighlight]    = 
        //style.Colors[ImGuiCol_NavWindowingDimBg]        = 
        //style.Colors[ImGuiCol_ModalWindowDimBg]         = 
        break;
#pragma endregion
#pragma region NIXWARE (WIP)
    case 10:
        if (applyStyle)
        {
            style.Alpha                     = 1.f;
            style.WindowPadding             = ImVec2(8, 8);
            style.WindowRounding            = 0.f;
            style.WindowTitleAlign          = ImVec2(0.5f, 0.5f);
            style.FramePadding              = ImVec2(4, 1);
            style.FrameRounding             = 0.f;
            style.ItemSpacing               = ImVec2(8, 4);
            style.ItemInnerSpacing          = ImVec2(4, 4);
            style.TouchExtraPadding         = ImVec2(0, 0);
            style.IndentSpacing             = 21.f;
            style.ColumnsMinSpacing         = 6.f;
            style.ScrollbarSize             = 10.f;
            style.ScrollbarRounding         = 3.f;
            style.GrabMinSize               = 10.f;
            style.GrabRounding              = 0.f;
            style.ButtonTextAlign           = ImVec2(0.5f, 0.5f);
            style.DisplayWindowPadding      = ImVec2(22, 22);
            style.DisplaySafeAreaPadding    = ImVec2(4, 4);
            style.AntiAliasedLines          = true;
            style.CurveTessellationTol      = 1.25f;
        }

        style.Colors[ImGuiCol_Text]                     = ImColor(255, 255, 255, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(61, 59, 74, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(0, 0, 0, 255);
        style.Colors[ImGuiCol_ChildBg]                  = ImColor(0, 0, 0, 255);
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(18, 18, 23, 255);
        style.Colors[ImGuiCol_Border]                   = ImColor(18, 18, 18, 255);
        style.Colors[ImGuiCol_BorderShadow]             = ImColor(235, 232, 224, 0);
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(18, 18, 18, 255);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(18, 18, 18, 255);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(18, 18, 18, 255);
        style.Colors[ImGuiCol_TitleBg]                  = ImColor(0, 0, 0, 255);
        style.Colors[ImGuiCol_TitleBgActive]            = ImColor(0, 0, 0, 255);
        style.Colors[ImGuiCol_TitleBgCollapsed]         = ImColor(0, 0, 0, 255);
        style.Colors[ImGuiCol_MenuBarBg]                = ImColor(26, 23, 31, 255);
        style.Colors[ImGuiCol_ScrollbarBg]              = ImColor(26, 23, 31, 255);
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(204, 204, 212, 79);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(143, 143, 148, 255);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = ImColor(15, 13, 18, 255);
        style.Colors[ImGuiCol_CheckMark]                = ImColor(204, 204, 212, 79);
        style.Colors[ImGuiCol_SliderGrab]               = ImColor(204, 204, 212, 79);
        style.Colors[ImGuiCol_SliderGrabActive]         = ImColor(15, 13, 18, 255);
        style.Colors[ImGuiCol_Button]                   = ImColor(0, 0, 0, 255);
        style.Colors[ImGuiCol_ButtonHovered]            = ImColor(61, 59, 74, 255);
        style.Colors[ImGuiCol_ButtonActive]             = ImColor(143, 143, 148, 255);
        style.Colors[ImGuiCol_Header]                   = ImColor(26, 23, 31, 255);
        style.Colors[ImGuiCol_HeaderHovered]            = ImColor(143, 143, 148, 255);
        style.Colors[ImGuiCol_HeaderActive]             = ImColor(15, 13, 18, 255);
        style.Colors[ImGuiCol_Separator]                = ImColor(143, 143, 148, 255);
        style.Colors[ImGuiCol_SeparatorHovered]         = ImColor(61, 59, 74, 255);
        style.Colors[ImGuiCol_SeparatorActive]          = ImColor(143, 143, 148, 255);
        style.Colors[ImGuiCol_ResizeGrip]               = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_ResizeGripHovered]        = ImColor(143, 143, 148, 255);
        style.Colors[ImGuiCol_ResizeGripActive]         = ImColor(15, 13, 18, 255);
    	if (applyLerp)
    	{
            style.Colors[ImGuiCol_Tab]                      = ImLerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 0.8f);
            style.Colors[ImGuiCol_TabHovered]               = style.Colors[ImGuiCol_HeaderHovered];
            style.Colors[ImGuiCol_TabActive]                = ImLerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.6f);
            style.Colors[ImGuiCol_TabUnfocused]             = ImLerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.8f);
            style.Colors[ImGuiCol_TabUnfocusedActive]       = ImLerp(style.Colors[ImGuiCol_TabActive], style.Colors[ImGuiCol_TitleBg], 0.4f);
    	}
        style.Colors[ImGuiCol_PlotLines]                = ImColor(102, 99, 97, 161);
        style.Colors[ImGuiCol_PlotLinesHovered]         = ImColor(64, 255, 0, 255);
        style.Colors[ImGuiCol_PlotHistogram]            = ImColor(102, 99, 97, 161);
        style.Colors[ImGuiCol_PlotHistogramHovered]     = ImColor(64, 255, 0, 255);
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(64, 255, 0, 110);
        //style.Colors[ImGuiCol_DragDropTarget]           = 
        //style.Colors[ImGuiCol_NavHighlight]             = 
        //style.Colors[ImGuiCol_NavWindowingHighlight]    = 
        //style.Colors[ImGuiCol_NavWindowingDimBg]        = 
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImColor(255, 250, 242, 186);
    	break;
#pragma endregion
#pragma region Sensum (Light) (WIP)
    case 11:
        if (applyStyle)
        {
            style.Alpha             = 1.f;
            style.WindowRounding    = 0.f;
            style.WindowBorderSize  = 0.f;
            style.FrameRounding     = 0.f;
            style.ScrollbarRounding = 0.f;
            style.ScrollbarSize     = 12.f;
            style.WindowTitleAlign  = ImVec2(0.5f, 0.5f);
            style.FramePadding      = ImVec2(4.f, 2.f);
            style.ItemSpacing       = ImVec2(5, 5);
            style.FrameRounding     = 2.f;
            style.WindowBorderSize  = 0.f;
            style.PopupBorderSize   = 0.5f;
            style.ItemSpacing       = ImVec2(4.f, 6.f);
            style.ItemInnerSpacing  = ImVec2(4.f, 4.f);
            style.GrabMinSize       = 16.f;
            style.FramePadding      = ImVec2(4.f, 1.f);
            style.GrabMinSize       = 1.f;
        }
    	
        style.Colors[ImGuiCol_Text]                     = ImColor(0, 0, 0, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(153, 153, 153, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(240, 240, 240, 255);
        style.Colors[ImGuiCol_ChildBg]                  = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(235, 235, 235, 250);
        style.Colors[ImGuiCol_Border]                   = ImColor(0, 0, 0, 76);
        style.Colors[ImGuiCol_BorderShadow]             = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(255, 255, 255, 255);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(204, 26, 26, 102);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(204, 26, 26, 171);
        style.Colors[ImGuiCol_TitleBg]                  = ImColor(15, 15, 15, 255);
        style.Colors[ImGuiCol_TitleBgActive]            = ImColor(15, 15, 15, 255);
        style.Colors[ImGuiCol_TitleBgCollapsed]         = ImColor(15, 15, 15, 255);
        style.Colors[ImGuiCol_MenuBarBg]                = ImColor(31, 31, 31, 255);
        style.Colors[ImGuiCol_ScrollbarBg]              = ImColor(250, 250, 250, 135);
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(23, 23, 23, 255);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(204, 26, 26, 255);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = ImColor(204, 26, 26, 255);
        style.Colors[ImGuiCol_CheckMark]                = ImColor(76, 76, 76, 255);
        style.Colors[ImGuiCol_SliderGrab]               = ImColor(178, 178, 178, 178);
        style.Colors[ImGuiCol_SliderGrabActive]         = ImColor(178, 178, 178, 255);
        style.Colors[ImGuiCol_Button]                   = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_ButtonHovered]            = ImColor(26, 26, 26, 51);
        style.Colors[ImGuiCol_ButtonActive]             = ImColor(26, 26, 26, 51);
        style.Colors[ImGuiCol_Header]                   = ImColor(204, 26, 26, 128);
        style.Colors[ImGuiCol_HeaderHovered]            = ImColor(204, 26, 26, 102);
        style.Colors[ImGuiCol_HeaderActive]             = ImColor(204, 26, 26, 171);
        //style.Colors[ImGuiCol_Header] = ImColor(204, 204, 204, 255);
        //style.Colors[ImGuiCol_HeaderHovered] = ImColor(204, 204, 204, 255);
        //style.Colors[ImGuiCol_HeaderActive] = ImColor(204, 204, 204, 255);
        style.Colors[ImGuiCol_Separator]                = ImColor(99, 99, 99, 255);
        style.Colors[ImGuiCol_SeparatorHovered]         = ImColor(36, 112, 204, 199);
        style.Colors[ImGuiCol_SeparatorActive]          = ImColor(36, 112, 204, 255);
        style.Colors[ImGuiCol_ResizeGrip]               = ImColor(204, 204, 204, 143);
        style.Colors[ImGuiCol_ResizeGripHovered]        = ImColor(66, 150, 250, 171);
        style.Colors[ImGuiCol_ResizeGripActive]         = ImColor(66, 150, 250, 242);
        if (applyLerp)
        {
            style.Colors[ImGuiCol_Tab]                      = ImLerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 0.8f);
            style.Colors[ImGuiCol_TabHovered]               = style.Colors[ImGuiCol_HeaderHovered];
            style.Colors[ImGuiCol_TabActive]                = ImLerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.6f);
            style.Colors[ImGuiCol_TabUnfocused]             = ImLerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.8f);
            style.Colors[ImGuiCol_TabUnfocusedActive]       = ImLerp(style.Colors[ImGuiCol_TabActive], style.Colors[ImGuiCol_TitleBg], 0.4f);
        }
        style.Colors[ImGuiCol_PlotLines]                = ImColor(99, 99, 99, 255);
        style.Colors[ImGuiCol_PlotLinesHovered]         = ImColor(255, 110, 89, 255);
        style.Colors[ImGuiCol_PlotHistogram]            = ImColor(230, 178, 0, 255);
        style.Colors[ImGuiCol_PlotHistogramHovered]     = ImColor(255, 115, 0, 255);
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(66, 150, 250, 89);
        style.Colors[ImGuiCol_DragDropTarget]           = ImColor(66, 150, 250, 242);
        //style.Colors[ImGuiCol_NavHighlight]             = 
        //style.Colors[ImGuiCol_NavWindowingHighlight]    = 
        //style.Colors[ImGuiCol_NavWindowingDimBg]        = 
        style.Colors[ImGuiCol_ModalWindowDimBg]         = ImColor(51, 51, 51, 89);
        break;
#pragma endregion
#pragma region Sensum (Dark) (WIP)
    case 12:
        if (applyStyle)
        {
            style.Alpha             = 0.95f;
            style.WindowRounding    = 0.f;
            style.WindowBorderSize  = 0.f;
            style.FrameRounding     = 0.f;
            style.ScrollbarRounding = 0.f;
            style.ScrollbarSize     = 12.f;
            style.WindowTitleAlign  = ImVec2(0.5f, 0.5f);
            style.FramePadding      = ImVec2(4.f, 2.f);
            style.ItemSpacing       = ImVec2(5, 5);
            style.FrameRounding     = 2.f;
            style.WindowBorderSize  = 0.f;
            style.PopupBorderSize   = 0.5f;
            style.ItemSpacing       = ImVec2(4.f, 6.f);
            style.ItemInnerSpacing  = ImVec2(4.f, 4.f);
            style.GrabMinSize       = 16.f;
            style.FramePadding      = ImVec2(4.f, 1.f);
            style.GrabMinSize       = 1.f;
        }

        style.Colors[ImGuiCol_Text]                     = ImColor(245, 245, 245, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(153, 153, 153, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(31, 31, 31, 250);
        style.Colors[ImGuiCol_ChildBg]                  = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(51, 51, 51, 250);
        style.Colors[ImGuiCol_Border]                   = ImColor(0, 0, 0, 76);
        style.Colors[ImGuiCol_BorderShadow]             = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(41, 41, 41, 255);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(51, 51, 51, 102);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(66, 66, 66, 171);
        style.Colors[ImGuiCol_TitleBg]                  = ImColor(15, 15, 15, 255);
        style.Colors[ImGuiCol_TitleBgActive]            = ImColor(15, 15, 15, 255);
        style.Colors[ImGuiCol_TitleBgCollapsed]         = ImColor(15, 15, 15, 255);
        style.Colors[ImGuiCol_MenuBarBg]                = ImColor(31, 31, 31, 255);
        style.Colors[ImGuiCol_ScrollbarBg]              = ImColor(250, 250, 250, 135);
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(23, 23, 23, 255);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(204, 26, 26, 255);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = ImColor(204, 26, 26, 255);
        style.Colors[ImGuiCol_CheckMark]                = ImColor(166, 26, 26, 255);
        style.Colors[ImGuiCol_SliderGrab]               = ImColor(66, 66, 66, 204);
        style.Colors[ImGuiCol_SliderGrabActive]         = ImColor(66, 66, 66, 255);
        style.Colors[ImGuiCol_Button]                   = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_ButtonHovered]            = ImColor(204, 204, 204, 51);
        style.Colors[ImGuiCol_ButtonActive]             = ImColor(204, 204, 204, 51);
        style.Colors[ImGuiCol_Header]                   = ImColor(204, 26, 26, 128);
        style.Colors[ImGuiCol_HeaderHovered]            = ImColor(204, 26, 26, 102);
        style.Colors[ImGuiCol_HeaderActive]             = ImColor(204, 26, 26, 171);
        //style.Colors[ImGuiCol_Header]                   = ImColor(25, 25, 25, 255);
        //style.Colors[ImGuiCol_HeaderHovered]            = ImColor(66, 66, 66, 255);
        //style.Colors[ImGuiCol_HeaderActive]             = ImColor(51, 51, 51, 255);
        style.Colors[ImGuiCol_Separator]                = ImColor(99, 99, 99, 255);
        style.Colors[ImGuiCol_SeparatorHovered]         = ImColor(36, 112, 204, 199);
        style.Colors[ImGuiCol_SeparatorActive]          = ImColor(36, 112, 204, 255);
        style.Colors[ImGuiCol_ResizeGrip]               = ImColor(204, 204, 204, 143);
        style.Colors[ImGuiCol_ResizeGripHovered]        = ImColor(66, 150, 250, 171);
        style.Colors[ImGuiCol_ResizeGripActive]         = ImColor(66, 150, 250, 242);
        if (applyLerp)
        {
            style.Colors[ImGuiCol_Tab]                      = ImLerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 0.8f);
            style.Colors[ImGuiCol_TabHovered]               = style.Colors[ImGuiCol_HeaderHovered];
            style.Colors[ImGuiCol_TabActive]                = ImLerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.6f);
            style.Colors[ImGuiCol_TabUnfocused]             = ImLerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.8f);
            style.Colors[ImGuiCol_TabUnfocusedActive]       = ImLerp(style.Colors[ImGuiCol_TabActive], style.Colors[ImGuiCol_TitleBg], 0.4f);
        }
        style.Colors[ImGuiCol_PlotLines]                = ImColor(99, 99, 99, 255);
        style.Colors[ImGuiCol_PlotLinesHovered]         = ImColor(255, 110, 89, 255);
        style.Colors[ImGuiCol_PlotHistogram]            = ImColor(230, 178, 0, 255);
        style.Colors[ImGuiCol_PlotHistogramHovered]     = ImColor(255, 115, 0, 255);
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(66, 150, 250, 89);
        style.Colors[ImGuiCol_DragDropTarget]           = ImColor(66, 150, 250, 242);
        //style.Colors[ImGuiCol_NavHighlight]             = 
        //style.Colors[ImGuiCol_NavWindowingHighlight]    = 
        //style.Colors[ImGuiCol_NavWindowingDimBg]        = 
        style.Colors[ImGuiCol_ModalWindowDimBg]         = ImColor(51, 51, 51, 89);
        break;
#pragma endregion
#pragma region Setupim (WIP)
    case 13:
        if (applyStyle)
        {
            style.Alpha = 1.f;
            style.FrameRounding = 3.f;
        }

        style.Colors[ImGuiCol_Text]                     = ImColor(0, 0, 0, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(153, 153, 153, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(240, 240, 240, 240);
        style.Colors[ImGuiCol_ChildBg]                  = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(255, 255, 255, 240);
        style.Colors[ImGuiCol_Border]                   = ImColor(0, 0, 0, 99);
        style.Colors[ImGuiCol_BorderShadow]             = ImColor(255, 255, 255, 26);
        style.Colors[ImGuiCol_FrameBg]                  = style.Colors[ImGuiCol_PopupBg];
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(66, 150, 250, 102);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(66, 150, 250, 171);
        style.Colors[ImGuiCol_TitleBg]                  = ImColor(245, 245, 245, 255);
        style.Colors[ImGuiCol_TitleBgActive]            = ImColor(209, 209, 209, 255);
        style.Colors[ImGuiCol_TitleBgCollapsed]         = ImColor(255, 255, 255, 130);
        style.Colors[ImGuiCol_MenuBarBg]                = ImColor(219, 219, 219, 255);
        style.Colors[ImGuiCol_ScrollbarBg]              = ImColor(250, 250, 250, 135);
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(176, 176, 176, 255);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(150, 150, 150, 255);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = ImColor(125, 125, 125, 255);
        style.Colors[ImGuiCol_CheckMark]                = ImColor(66, 150, 250, 255);
        style.Colors[ImGuiCol_SliderGrab]               = ImColor(61, 133, 224, 255);
        style.Colors[ImGuiCol_SliderGrabActive]         = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_Button]                   = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_ButtonHovered]            = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_ButtonActive]             = ImColor(15, 135, 250, 255);
        style.Colors[ImGuiCol_Header]                   = ImColor(66, 150, 250, 79);
        style.Colors[ImGuiCol_HeaderHovered]            = ImColor(66, 150, 250, 204);
        style.Colors[ImGuiCol_HeaderActive]             = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_Separator]                = ImColor(99, 99, 99, 255);
        style.Colors[ImGuiCol_SeparatorHovered]         = ImColor(66, 150, 250, 199);
        style.Colors[ImGuiCol_SeparatorActive]          = style.Colors[ImGuiCol_CheckMark];
    	style.Colors[ImGuiCol_ResizeGrip]               = ImColor(255, 255, 255, 128);
        style.Colors[ImGuiCol_ResizeGripHovered]        = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_ResizeGripActive]         = ImColor(66, 150, 250, 242);
        if (applyLerp)
        {
            style.Colors[ImGuiCol_Tab]                      = ImLerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 0.8f);
            style.Colors[ImGuiCol_TabHovered]               = style.Colors[ImGuiCol_HeaderHovered];
            style.Colors[ImGuiCol_TabActive]                = ImLerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.6f);
            style.Colors[ImGuiCol_TabUnfocused]             = ImLerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.8f);
            style.Colors[ImGuiCol_TabUnfocusedActive]       = ImLerp(style.Colors[ImGuiCol_TabActive], style.Colors[ImGuiCol_TitleBg], 0.4f);
        }
        style.Colors[ImGuiCol_PlotLines]                = style.Colors[ImGuiCol_Separator];
        style.Colors[ImGuiCol_PlotLinesHovered]         = ImColor(255, 110, 89, 255);
        style.Colors[ImGuiCol_PlotHistogram]            = ImColor(230, 178, 0, 255);
        style.Colors[ImGuiCol_PlotHistogramHovered]     = ImColor(255, 153, 0, 255);
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(66, 150, 250, 89);
        //style.Colors[ImGuiCol_DragDropTarget]           = 
        //style.Colors[ImGuiCol_NavHighlight]             = 
        //style.Colors[ImGuiCol_NavWindowingHighlight]    = 
        //style.Colors[ImGuiCol_NavWindowingDimBg]        = 
        style.Colors[ImGuiCol_ModalWindowDimBg]         = ImColor(51, 51, 51, 89);
        break;
#pragma endregion
#pragma region Monochrome (WIP)
    case 14:
        if (applyStyle)
        {
            style.WindowRounding    = 5.3f;
            style.FrameRounding     = 2.3f;
            style.ScrollbarRounding = 0.f;
        }

        style.Colors[ImGuiCol_Text]                     = ImColor(230, 230, 230, 230);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(153, 153, 153, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(23, 23, 38, 255);
        style.Colors[ImGuiCol_ChildBg]                  = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(13, 13, 26, 217);
        style.Colors[ImGuiCol_Border]                   = ImColor(178, 178, 178, 166);
        style.Colors[ImGuiCol_BorderShadow]             = style.Colors[ImGuiCol_ChildBg];
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(0, 0, 3, 255);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(230, 204, 204, 102);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(230, 166, 166, 115);
        style.Colors[ImGuiCol_TitleBg]                  = ImColor(0, 0, 0, 212);
        style.Colors[ImGuiCol_TitleBgActive]            = ImColor(0, 0, 0, 222);
        style.Colors[ImGuiCol_TitleBgCollapsed]         = ImColor(102, 102, 204, 51);
        style.Colors[ImGuiCol_MenuBarBg]                = ImColor(3, 3, 5, 204);
        style.Colors[ImGuiCol_ScrollbarBg]              = ImColor(51, 64, 76, 153);
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(140, 135, 140, 130);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(143, 143, 143, 255);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = ImColor(143, 143, 143, 232);
        style.Colors[ImGuiCol_CheckMark]                = ImColor(230, 230, 230, 212);
        style.Colors[ImGuiCol_SliderGrab]               = ImColor(178, 178, 178, 158);
        style.Colors[ImGuiCol_SliderGrabActive]         = ImColor(76, 76, 76, 214);
        style.Colors[ImGuiCol_Button]                   = ImColor(122, 184, 227, 125);
        style.Colors[ImGuiCol_ButtonHovered]            = ImColor(128, 176, 252, 173);
        style.Colors[ImGuiCol_ButtonActive]             = ImColor(204, 128, 128, 255);
        style.Colors[ImGuiCol_Header]                   = ImColor(76, 176, 255, 135);
        style.Colors[ImGuiCol_HeaderHovered]            = ImColor(112, 156, 219, 255);
        style.Colors[ImGuiCol_HeaderActive]             = ImColor(97, 158, 212, 255);
        style.Colors[ImGuiCol_Separator]                = ImColor(128, 128, 128, 255);
        style.Colors[ImGuiCol_SeparatorHovered]         = ImColor(178, 153, 153, 255);
        style.Colors[ImGuiCol_SeparatorActive]          = ImColor(230, 178, 178, 255);
        style.Colors[ImGuiCol_ResizeGrip]               = ImColor(255, 255, 255, 217);
        style.Colors[ImGuiCol_ResizeGripHovered]        = ImColor(255, 255, 255, 153);
        style.Colors[ImGuiCol_ResizeGripActive]         = ImColor(255, 255, 255, 230);
        if (applyLerp)
        {
            style.Colors[ImGuiCol_Tab]                      = ImLerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 0.8f);
            style.Colors[ImGuiCol_TabHovered]               = style.Colors[ImGuiCol_HeaderHovered];
            style.Colors[ImGuiCol_TabActive]                = ImLerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.6f);
            style.Colors[ImGuiCol_TabUnfocused]             = ImLerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.8f);
            style.Colors[ImGuiCol_TabUnfocusedActive]       = ImLerp(style.Colors[ImGuiCol_TabActive], style.Colors[ImGuiCol_TitleBg], 0.4f);
        }
        style.Colors[ImGuiCol_PlotLines]                = ImColor(255, 255, 255, 255);
        style.Colors[ImGuiCol_PlotLinesHovered]         = ImColor(230, 178, 0, 255);
        style.Colors[ImGuiCol_PlotHistogram]            = style.Colors[ImGuiCol_PlotLinesHovered];
        style.Colors[ImGuiCol_PlotHistogramHovered]     = ImColor(255, 153, 0, 255);
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(0, 0, 255, 89);
        //style.Colors[ImGuiCol_DragDropTarget]           = 
        //style.Colors[ImGuiCol_NavHighlight]             = 
        //style.Colors[ImGuiCol_NavWindowingHighlight]    = 
        //style.Colors[ImGuiCol_NavWindowingDimBg]        = 
        style.Colors[ImGuiCol_ModalWindowDimBg]         = ImColor(51, 51, 51, 89);
    	break;
#pragma endregion
#pragma region Procedural (WIP)
    case 15:
        style.Colors[ImGuiCol_Text]                     = ImColor(236, 240, 241, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(236, 240, 241, 148);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(44, 62, 80, 242);
        style.Colors[ImGuiCol_ChildBg]                  = ImColor(57, 79, 105, 148);
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(33, 46, 60, 235);
        style.Colors[ImGuiCol_Border]                   = ImColor(44, 62, 80, 0);
        style.Colors[ImGuiCol_BorderShadow]             = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(57, 79, 105, 255);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(41, 128, 185, 199);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(41, 128, 185, 255);
        style.Colors[ImGuiCol_TitleBg]                  = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_TitleBgActive]            = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_TitleBgCollapsed]         = ImColor(57, 79, 105, 191);
        style.Colors[ImGuiCol_MenuBarBg]                = ImColor(57, 79, 105, 120);
        style.Colors[ImGuiCol_ScrollbarBg]              = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(41, 128, 185, 54);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_CheckMark]                = ImColor(41, 128, 185, 204);
        style.Colors[ImGuiCol_SliderGrab]               = ImColor(41, 128, 185, 127);
        style.Colors[ImGuiCol_SliderGrabActive]         = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Button]                   = style.Colors[ImGuiCol_SliderGrab];
        style.Colors[ImGuiCol_ButtonHovered]            = ImColor(41, 128, 185, 219);
        style.Colors[ImGuiCol_ButtonActive]             = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Header]                   = ImColor(41, 128, 185, 194);
        style.Colors[ImGuiCol_HeaderHovered]            = style.Colors[ImGuiCol_ButtonHovered];
        style.Colors[ImGuiCol_HeaderActive]             = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Separator]                = ImColor(41, 128, 185, 82);
        style.Colors[ImGuiCol_SeparatorHovered]         = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_SeparatorActive]          = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_ResizeGrip]               = ImColor(41, 128, 185, 38);
        style.Colors[ImGuiCol_ResizeGripHovered]        = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_ResizeGripActive]         = style.Colors[ImGuiCol_FrameBgActive];
        if (applyLerp)
        {
            style.Colors[ImGuiCol_Tab]                      = ImLerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 0.8f);
            style.Colors[ImGuiCol_TabHovered]               = style.Colors[ImGuiCol_HeaderHovered];
            style.Colors[ImGuiCol_TabActive]                = ImLerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.6f);
            style.Colors[ImGuiCol_TabUnfocused]             = ImLerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.8f);
            style.Colors[ImGuiCol_TabUnfocusedActive]       = ImLerp(style.Colors[ImGuiCol_TabActive], style.Colors[ImGuiCol_TitleBg], 0.4f);
        }
        style.Colors[ImGuiCol_PlotLines]                = ImColor(236, 240, 241, 161);
        style.Colors[ImGuiCol_PlotLinesHovered]         = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_PlotHistogram]            = style.Colors[ImGuiCol_PlotLines];
        style.Colors[ImGuiCol_PlotHistogramHovered]     = style.Colors[ImGuiCol_FrameBgActive];
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(41, 128, 185, 110);
        //style.Colors[ImGuiCol_DragDropTarget]           = 
        //style.Colors[ImGuiCol_NavHighlight]             = 
        //style.Colors[ImGuiCol_NavWindowingHighlight]    = 
        //style.Colors[ImGuiCol_NavWindowingDimBg]        = 
        style.Colors[ImGuiCol_ModalWindowDimBg]         = ImColor(57, 79, 105, 186);
    	break;
#pragma endregion
#pragma region Pagghiu (WIP)
    case 16:
        if (applyStyle)
        {
            style.Alpha         = 1.f;
            style.FrameRounding = 4;
            style.IndentSpacing = 12.f;
        }

        style.Colors[ImGuiCol_Text]                     = ImColor(79, 64, 61, 255);
        //style.Colors[ImGuiCol_TextDisabled]             = 
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(240, 240, 240, 255);
        style.Colors[ImGuiCol_ChildBg]                  = ImColor(173, 173, 173, 0);
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(209, 235, 255, 230);
        style.Colors[ImGuiCol_Border]                   = ImColor(128, 128, 128, 153);
        style.Colors[ImGuiCol_BorderShadow]             = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(158, 178, 184, 143);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(242, 84, 36, 120);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(247, 79, 33, 207);
        style.Colors[ImGuiCol_TitleBg]                  = ImColor(107, 191, 255, 135);
        //style.Colors[ImGuiCol_TitleBgActive]            = 
        style.Colors[ImGuiCol_TitleBgCollapsed]         = ImColor(102, 166, 204, 51);
        style.Colors[ImGuiCol_MenuBarBg]                = ImColor(189, 189, 240, 255);
        style.Colors[ImGuiCol_ScrollbarBg]              = ImColor(102, 158, 204, 38);
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(99, 163, 204, 76);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(71, 171, 204, 150);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = ImColor(64, 122, 135, 171);
        style.Colors[ImGuiCol_CheckMark]                = ImColor(122, 120, 120, 181);
        //style.Colors[ImGuiCol_SliderGrab]               = 
        style.Colors[ImGuiCol_SliderGrabActive]         = ImColor(79, 120, 252, 255);
        style.Colors[ImGuiCol_Button]                   = ImColor(255, 201, 46, 199);
        style.Colors[ImGuiCol_ButtonHovered]            = ImColor(107, 209, 255, 207);
        style.Colors[ImGuiCol_ButtonActive]             = ImColor(184, 255, 255, 219);
        style.Colors[ImGuiCol_Header]                   = ImColor(166, 199, 214, 204);
        style.Colors[ImGuiCol_HeaderHovered]            = ImColor(191, 224, 240, 204);
        style.Colors[ImGuiCol_HeaderActive]             = ImColor(140, 173, 189, 204);
        //style.Colors[ImGuiCol_Separator]                = 
        //style.Colors[ImGuiCol_SeparatorHovered]         = 
        //style.Colors[ImGuiCol_SeparatorActive]          = 
        style.Colors[ImGuiCol_ResizeGrip]               = ImColor(153, 153, 204, 76);
        style.Colors[ImGuiCol_ResizeGripHovered]        = ImColor(255, 255, 255, 153);
        style.Colors[ImGuiCol_ResizeGripActive]         = ImColor(255, 255, 255, 230);
        /*if (applyLerp)
        {
            style.Colors[ImGuiCol_Tab]                      = ImLerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 0.8f);
            style.Colors[ImGuiCol_TabHovered]               = style.Colors[ImGuiCol_HeaderHovered];
            style.Colors[ImGuiCol_TabActive]                = ImLerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.6f);
            style.Colors[ImGuiCol_TabUnfocused]             = ImLerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.8f);
            style.Colors[ImGuiCol_TabUnfocusedActive]       = ImLerp(style.Colors[ImGuiCol_TabActive], style.Colors[ImGuiCol_TitleBg], 0.4f);
        }*/
        //style.Colors[ImGuiCol_PlotLines]                = 
        //style.Colors[ImGuiCol_PlotLinesHovered]         = 
        //style.Colors[ImGuiCol_PlotHistogram]            = 
        //style.Colors[ImGuiCol_PlotHistogramHovered]     = 
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(255, 252, 138, 110);
        //style.Colors[ImGuiCol_DragDropTarget]           = 
        //style.Colors[ImGuiCol_NavHighlight]             = 
        //style.Colors[ImGuiCol_NavWindowingHighlight]    = 
        //style.Colors[ImGuiCol_NavWindowingDimBg]        = 
        //style.Colors[ImGuiCol_ModalWindowDimBg]         = 
        break;
#pragma endregion
#pragma region Doug (WIP)
    case 17:
        style.Colors[ImGuiCol_Text]                     = ImColor(255, 255, 255, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(102, 102, 102, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(15, 15, 15, 240);
        style.Colors[ImGuiCol_ChildBg]                  = ImColor(255, 255, 255, 0);
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(0, 0, 0, 240);
        style.Colors[ImGuiCol_Border]                   = ImColor(255, 255, 255, 48);
        //style.Colors[ImGuiCol_BorderShadow]             = 
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(41, 74, 122, 138);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(66, 150, 250, 102);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(66, 150, 250, 171);
        style.Colors[ImGuiCol_TitleBg]                  = ImColor(10, 10, 10, 255);
        style.Colors[ImGuiCol_TitleBgActive]            = ImColor(46, 46, 46, 255);
        style.Colors[ImGuiCol_TitleBgCollapsed]         = ImColor(0, 0, 0, 130);
        style.Colors[ImGuiCol_MenuBarBg]                = ImColor(36, 36, 36, 255);
        style.Colors[ImGuiCol_ScrollbarBg]              = ImColor(5, 5, 5, 135);
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(79, 79, 79, 255);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(105, 105, 105, 255);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = ImColor(130, 130, 130, 255);
        style.Colors[ImGuiCol_CheckMark]                = ImColor(66, 150, 250, 255);
        style.Colors[ImGuiCol_SliderGrab]               = ImColor(61, 133, 224, 255);
        style.Colors[ImGuiCol_SliderGrabActive]         = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_Button]                   = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_ButtonHovered]            = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_ButtonActive]             = ImColor(15, 135, 250, 255);
        style.Colors[ImGuiCol_Header]                   = ImColor(66, 150, 250, 79);
        style.Colors[ImGuiCol_HeaderHovered]            = ImColor(66, 150, 250, 204);
        style.Colors[ImGuiCol_HeaderActive]             = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_Separator]                = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_SeparatorHovered]         = ImColor(66, 150, 250, 199);
        style.Colors[ImGuiCol_SeparatorActive]          = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_ResizeGrip]               = ImColor(66, 150, 250, 64);
        style.Colors[ImGuiCol_ResizeGripHovered]        = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_ResizeGripActive]         = ImColor(66, 150, 250, 242);
    	if (applyLerp)
        {
            style.Colors[ImGuiCol_Tab]                      = ImLerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 0.8f);
            style.Colors[ImGuiCol_TabHovered]               = style.Colors[ImGuiCol_HeaderHovered];
            style.Colors[ImGuiCol_TabActive]                = ImLerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.6f);
            style.Colors[ImGuiCol_TabUnfocused]             = ImLerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.8f);
            style.Colors[ImGuiCol_TabUnfocusedActive]       = ImLerp(style.Colors[ImGuiCol_TabActive], style.Colors[ImGuiCol_TitleBg], 0.4f);
        }
        style.Colors[ImGuiCol_PlotLines]                = ImColor(156, 156, 156, 255);
        style.Colors[ImGuiCol_PlotLinesHovered]         = ImColor(255, 110, 89, 255);
        style.Colors[ImGuiCol_PlotHistogram]            = ImColor(230, 178, 0, 255);
        style.Colors[ImGuiCol_PlotHistogramHovered]     = ImColor(255, 153, 0, 255);
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(66, 150, 250, 89);
        //style.Colors[ImGuiCol_DragDropTarget]           = 
        style.Colors[ImGuiCol_NavHighlight]             = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_NavWindowingHighlight]    = ImColor(255, 255, 255, 31);
        //style.Colors[ImGuiCol_NavWindowingDimBg]        = 
        style.Colors[ImGuiCol_ModalWindowDimBg]         = ImColor(204, 204, 204, 89);
        break;
#pragma endregion
#pragma region Microsoft (WIP)
    case 18:
        if (applyStyle)
        {
            style.DisplaySafeAreaPadding    = ImVec2(0, 0);
            style.WindowPadding             = ImVec2(4, 6);
            style.FramePadding              = ImVec2(8, 6);
            style.ItemSpacing               = style.FramePadding;
            style.ItemInnerSpacing          = style.FramePadding;
            style.IndentSpacing             = 20.f;
            style.WindowRounding            = 0.f;
            style.FrameRounding             = style.WindowRounding;
            style.WindowBorderSize          = style.WindowRounding;
            style.FrameBorderSize           = 1.f;
            style.PopupBorderSize           = style.FrameBorderSize;
            style.ScrollbarSize             = 20.f;
            style.ScrollbarRounding         = style.WindowRounding;
            style.GrabMinSize               = 5.f;
            style.GrabRounding              = style.WindowRounding;
        }

        style.Colors[ImGuiCol_Text]                     = ImColor(26, 26, 26, 255);
        //style.Colors[ImGuiCol_TextDisabled]             = 
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(242, 242, 242, 255);
        style.Colors[ImGuiCol_ChildBg]                  = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(255, 255, 255, 255);
        style.Colors[ImGuiCol_Border]                   = ImColor(153, 153, 153, 255);
        style.Colors[ImGuiCol_BorderShadow]             = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_FrameBg]                  = style.Colors[ImGuiCol_PopupBg];
        style.Colors[ImGuiCol_FrameBgHovered]           = style.Colors[ImGuiCol_ButtonHovered];
        style.Colors[ImGuiCol_FrameBgActive]            = style.Colors[ImGuiCol_ButtonActive];
        //style.Colors[ImGuiCol_TitleBg]                  = 
        //style.Colors[ImGuiCol_TitleBgActive]            = 
        //style.Colors[ImGuiCol_TitleBgCollapsed]         = 
        style.Colors[ImGuiCol_MenuBarBg]                = style.Colors[ImGuiCol_Button];
        style.Colors[ImGuiCol_ScrollbarBg]              = style.Colors[ImGuiCol_Button];
        style.Colors[ImGuiCol_ScrollbarGrab]            = style.Colors[ImGuiCol_SliderGrab];
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(0, 0, 0, 51);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = style.Colors[ImGuiCol_SliderGrabActive];
        style.Colors[ImGuiCol_CheckMark]                = style.Colors[ImGuiCol_Text];
        style.Colors[ImGuiCol_SliderGrab]               = ImColor(176, 176, 176, 255);
        style.Colors[ImGuiCol_SliderGrabActive]         = ImColor(0, 0, 0, 128);
        style.Colors[ImGuiCol_Button]                   = ImColor(219, 219, 219, 255);
        style.Colors[ImGuiCol_ButtonHovered]            = ImColor(0, 120, 214, 51);
        style.Colors[ImGuiCol_ButtonActive]             = ImColor(0, 120, 214, 255);
        style.Colors[ImGuiCol_Header]                   = style.Colors[ImGuiCol_Button];
        style.Colors[ImGuiCol_HeaderHovered]            = style.Colors[ImGuiCol_ButtonHovered];
        style.Colors[ImGuiCol_HeaderActive]             = style.Colors[ImGuiCol_ButtonActive];
        //style.Colors[ImGuiCol_Separator]                = 
        //style.Colors[ImGuiCol_SeparatorHovered]         = 
        //style.Colors[ImGuiCol_SeparatorActive]          = 
        //style.Colors[ImGuiCol_ResizeGrip]               = 
        //style.Colors[ImGuiCol_ResizeGripHovered]        = 
        //style.Colors[ImGuiCol_ResizeGripActive]         = 
        /*if (applyLerp)
        {
            style.Colors[ImGuiCol_Tab]                      = ImLerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 0.8f);
            style.Colors[ImGuiCol_TabHovered]               = style.Colors[ImGuiCol_HeaderHovered];
            style.Colors[ImGuiCol_TabActive]                = ImLerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.6f);
            style.Colors[ImGuiCol_TabUnfocused]             = ImLerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.8f);
            style.Colors[ImGuiCol_TabUnfocusedActive]       = ImLerp(style.Colors[ImGuiCol_TabActive], style.Colors[ImGuiCol_TitleBg], 0.4f);
        }*/
        //style.Colors[ImGuiCol_PlotLines]                = 
        //style.Colors[ImGuiCol_PlotLinesHovered]         = 
        //style.Colors[ImGuiCol_PlotHistogram]            = 
        //style.Colors[ImGuiCol_PlotHistogramHovered]     = 
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        //style.Colors[ImGuiCol_TextSelectedBg]           = 
        //style.Colors[ImGuiCol_DragDropTarget]           = 
        //style.Colors[ImGuiCol_NavHighlight]             = 
        //style.Colors[ImGuiCol_NavWindowingHighlight]    =
        //style.Colors[ImGuiCol_NavWindowingDimBg]        =
    	//style.Colors[ImGuiCol_ModalWindowDimBg]         = 
        break;
#pragma endregion
#pragma region Darcula (WIP)
    case 19:
        if (applyStyle)
        {
            style.WindowRounding    = 5.3f;
            style.GrabRounding      = style.FrameRounding = 2.3f;
            style.FrameBorderSize   = 1.f;
            style.ScrollbarRounding = 5.f;
            style.ItemSpacing.y     = 6.5f;
        }

        style.Colors[ImGuiCol_Text]                     = ImColor(187, 187, 187, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(88, 88, 88, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(60, 63, 65, 240);
        style.Colors[ImGuiCol_ChildBg]                  = ImColor(60, 63, 65, 0);
        style.Colors[ImGuiCol_PopupBg]                  = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_Border]                   = ImColor(85, 85, 85, 128);
        style.Colors[ImGuiCol_BorderShadow]             = ImColor(40, 40, 40, 0);
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(43, 43, 43, 138);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(116, 172, 254, 171);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(120, 120, 120, 171);
        style.Colors[ImGuiCol_TitleBg]                  = ImColor(10, 10, 10, 255);
        style.Colors[ImGuiCol_TitleBgCollapsed]         = ImColor(41, 74, 122, 255);
        style.Colors[ImGuiCol_TitleBgActive]            = ImColor(0, 0, 0, 130);
        style.Colors[ImGuiCol_MenuBarBg]                = ImColor(69, 73, 74, 204);
        style.Colors[ImGuiCol_ScrollbarBg]              = ImColor(69, 73, 74, 153);
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(56, 79, 107, 130);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(56, 79, 107, 255);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = ImColor(35, 49, 67, 232);
        style.Colors[ImGuiCol_CheckMark]                = ImColor(230, 230, 230, 212);
        style.Colors[ImGuiCol_SliderGrab]               = ImColor(178, 178, 178, 158);
        style.Colors[ImGuiCol_SliderGrabActive]         = ImColor(76, 76, 76, 214);
        style.Colors[ImGuiCol_Button]                   = ImColor(85, 90, 92, 125);
        style.Colors[ImGuiCol_ButtonHovered]            = style.Colors[ImGuiCol_ScrollbarGrabHovered];
        style.Colors[ImGuiCol_ButtonActive]             = ImColor(35, 49, 67, 255);
        style.Colors[ImGuiCol_Header]                   = ImColor(85, 90, 92, 135);
        style.Colors[ImGuiCol_HeaderHovered]            = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_HeaderActive]             = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Separator]                = ImColor(81, 81, 81, 255);
        style.Colors[ImGuiCol_SeparatorHovered]         = style.Colors[ImGuiCol_Separator];
        style.Colors[ImGuiCol_SeparatorActive]          = style.Colors[ImGuiCol_Separator];
        style.Colors[ImGuiCol_ResizeGrip]               = ImColor(255, 255, 255, 217);
        style.Colors[ImGuiCol_ResizeGripHovered]        = ImColor(255, 255, 255, 153);
        style.Colors[ImGuiCol_ResizeGripActive]         = ImColor(255, 255, 255, 230);
    	if (applyLerp)
        {
            style.Colors[ImGuiCol_Tab]                      = ImLerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 0.8f);
            style.Colors[ImGuiCol_TabHovered]               = style.Colors[ImGuiCol_HeaderHovered];
            style.Colors[ImGuiCol_TabActive]                = ImLerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.6f);
            style.Colors[ImGuiCol_TabUnfocused]             = ImLerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.8f);
            style.Colors[ImGuiCol_TabUnfocusedActive]       = ImLerp(style.Colors[ImGuiCol_TabActive], style.Colors[ImGuiCol_TitleBg], 0.4f);
        }
        style.Colors[ImGuiCol_PlotLines]                = ImColor(156, 156, 156, 255);
        style.Colors[ImGuiCol_PlotLinesHovered]         = ImColor(255, 110, 89, 255);
        style.Colors[ImGuiCol_PlotHistogram]            = ImColor(230, 178, 0, 255);
        style.Colors[ImGuiCol_PlotHistogramHovered]     = ImColor(255, 153, 0, 255);
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(47, 101, 202, 230);
        //style.Colors[ImGuiCol_DragDropTarget]           = 
        //style.Colors[ImGuiCol_NavHighlight]             = 
        //style.Colors[ImGuiCol_NavWindowingHighlight]    =
        //style.Colors[ImGuiCol_NavWindowingDimBg]        =
        //style.Colors[ImGuiCol_ModalWindowDimBg]         = 
        break;
#pragma endregion
#pragma region UE4 (Unreal Engine 4) (WIP)
    case 20:
        style.Colors[ImGuiCol_Text]                     = ImColor(255, 255, 255, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(128, 128, 128, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(15, 15, 15, 240);
        style.Colors[ImGuiCol_ChildBg]                  = ImColor(255, 255, 255, 0);
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(20, 20, 20, 240);
        style.Colors[ImGuiCol_Border]                   = ImColor(110, 110, 128, 128);
        style.Colors[ImGuiCol_BorderShadow]             = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(51, 54, 56, 138);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(102, 102, 102, 102);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(46, 46, 46, 171);
        style.Colors[ImGuiCol_TitleBg]                  = ImColor(10, 10, 10, 255);
        style.Colors[ImGuiCol_TitleBgActive]            = ImColor(74, 74, 74, 255);
        style.Colors[ImGuiCol_TitleBgCollapsed]         = ImColor(0, 0, 0, 130);
        style.Colors[ImGuiCol_MenuBarBg]                = ImColor(36, 36, 36, 255);
        style.Colors[ImGuiCol_ScrollbarBg]              = ImColor(5, 5, 5, 135);
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(79, 79, 79, 255);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(105, 105, 105, 255);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = ImColor(130, 130, 130, 255);
        style.Colors[ImGuiCol_CheckMark]                = ImColor(240, 240, 240, 255);
        style.Colors[ImGuiCol_SliderGrab]               = style.Colors[ImGuiCol_ScrollbarGrabActive];
        style.Colors[ImGuiCol_SliderGrabActive]         = ImColor(219, 219, 219, 255);
        style.Colors[ImGuiCol_Button]                   = ImColor(112, 112, 112, 102);
        style.Colors[ImGuiCol_ButtonHovered]            = ImColor(117, 120, 122, 255);
        style.Colors[ImGuiCol_ButtonActive]             = ImColor(107, 107, 107, 255);
        style.Colors[ImGuiCol_Header]                   = ImColor(178, 178, 178, 79);
        style.Colors[ImGuiCol_HeaderHovered]            = ImColor(178, 178, 178, 204);
        style.Colors[ImGuiCol_HeaderActive]             = ImColor(122, 128, 133, 255);
        style.Colors[ImGuiCol_Separator]                = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_SeparatorHovered]         = ImColor(184, 184, 184, 199);
        style.Colors[ImGuiCol_SeparatorActive]          = style.Colors[ImGuiCol_ScrollbarGrabActive];
        style.Colors[ImGuiCol_ResizeGrip]               = ImColor(232, 232, 232, 64);
        style.Colors[ImGuiCol_ResizeGripHovered]        = ImColor(207, 207, 207, 171);
        style.Colors[ImGuiCol_ResizeGripActive]         = ImColor(117, 117, 117, 242);
    	if (applyLerp)
        {
            style.Colors[ImGuiCol_Tab]                      = ImLerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 0.8f);
            style.Colors[ImGuiCol_TabHovered]               = style.Colors[ImGuiCol_HeaderHovered];
            style.Colors[ImGuiCol_TabActive]                = ImLerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.6f);
            style.Colors[ImGuiCol_TabUnfocused]             = ImLerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.8f);
            style.Colors[ImGuiCol_TabUnfocusedActive]       = ImLerp(style.Colors[ImGuiCol_TabActive], style.Colors[ImGuiCol_TitleBg], 0.4f);
        }
        style.Colors[ImGuiCol_PlotLines]                = ImColor(156, 156, 156, 255);
        style.Colors[ImGuiCol_PlotLinesHovered]         = ImColor(255, 110, 89, 255);
        style.Colors[ImGuiCol_PlotHistogram]            = ImColor(186, 153, 38, 255);
        style.Colors[ImGuiCol_PlotHistogramHovered]     = ImColor(255, 153, 0, 255);
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(222, 222, 222, 89);
        style.Colors[ImGuiCol_DragDropTarget]           = ImColor(255, 255, 0, 230);
        style.Colors[ImGuiCol_NavHighlight]             = ImColor(153, 153, 153, 255);
        style.Colors[ImGuiCol_NavWindowingHighlight]    = ImColor(255, 255, 255, 178);
        //style.Colors[ImGuiCol_NavWindowingDimBg]        =
        style.Colors[ImGuiCol_ModalWindowDimBg]         = ImColor(204, 204, 204, 89);
    	break;
#pragma endregion
#pragma region Cherry (WIP)
    case 21:
        if (applyStyle)
        {
            style.WindowPadding         = ImVec2(6, 4);
            style.WindowRounding        = 0.f;
            style.WindowBorderSize      = 1.f;
            style.WindowTitleAlign.x    = 0.5f;
            style.FramePadding          = ImVec2(5, 2);
            style.FrameRounding         = 3.f;
            style.FrameBorderSize       = 0.f;
            style.ItemSpacing           = ImVec2(7, 1);
            style.ItemInnerSpacing      = ImVec2(1, 1);
            style.TouchExtraPadding     = ImVec2(0, 0);
            style.IndentSpacing         = 6.f;
            style.ScrollbarSize         = 12.f;
            style.ScrollbarRounding     = 16.f;
            style.GrabMinSize           = 20.f;
            style.GrabRounding          = 2.f;
        }

        style.Colors[ImGuiCol_Text]                     = ImColor(219, 237, 227, 199);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(219, 237, 227, 71);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(33, 36, 43, 255);
        style.Colors[ImGuiCol_ChildBg]                  = ImColor(51, 56, 69, 148);
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(51, 56, 69, 230);
        style.Colors[ImGuiCol_Border]                   = ImColor(137, 122, 65, 41);
        style.Colors[ImGuiCol_BorderShadow]             = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(51, 56, 69, 255);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(116, 50, 77, 199);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(116, 50, 77, 255);
        style.Colors[ImGuiCol_TitleBg]                  = ImColor(59, 51, 69, 255);
        style.Colors[ImGuiCol_TitleBgActive]            = ImColor(128, 19, 65, 255);
        style.Colors[ImGuiCol_TitleBgCollapsed]         = ImColor(51, 56, 69, 191);
        style.Colors[ImGuiCol_MenuBarBg]                = ImColor(51, 56, 69, 120);
        style.Colors[ImGuiCol_ScrollbarBg]              = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(23, 38, 41, 255);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_CheckMark]                = ImColor(181, 56, 69, 255);
        style.Colors[ImGuiCol_SliderGrab]               = ImColor(120, 196, 212, 36);
        style.Colors[ImGuiCol_SliderGrabActive]         = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_Button]                   = style.Colors[ImGuiCol_SliderGrab];
        style.Colors[ImGuiCol_ButtonHovered]            = ImColor(116, 50, 77, 219);
        style.Colors[ImGuiCol_ButtonActive]             = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Header]                   = ImColor(116, 50, 77, 194);
        style.Colors[ImGuiCol_HeaderHovered]            = style.Colors[ImGuiCol_ButtonHovered];
        style.Colors[ImGuiCol_HeaderActive]             = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_Separator]                = ImColor(36, 41, 48, 255);
        style.Colors[ImGuiCol_SeparatorHovered]         = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_SeparatorActive]          = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_ResizeGrip]               = ImColor(120, 196, 212, 10);
        style.Colors[ImGuiCol_ResizeGripHovered]        = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_ResizeGripActive]         = style.Colors[ImGuiCol_FrameBgActive];
    	if (applyLerp)
        {
            style.Colors[ImGuiCol_Tab]                      = ImLerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 0.8f);
            style.Colors[ImGuiCol_TabHovered]               = style.Colors[ImGuiCol_HeaderHovered];
            style.Colors[ImGuiCol_TabActive]                = ImLerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.6f);
            style.Colors[ImGuiCol_TabUnfocused]             = ImLerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.8f);
            style.Colors[ImGuiCol_TabUnfocusedActive]       = ImLerp(style.Colors[ImGuiCol_TabActive], style.Colors[ImGuiCol_TitleBg], 0.4f);
        }
        style.Colors[ImGuiCol_PlotLines]                = ImColor(219, 237, 227, 161);
        style.Colors[ImGuiCol_PlotLinesHovered]         = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_PlotHistogram]            = style.Colors[ImGuiCol_PlotLines];
        style.Colors[ImGuiCol_PlotHistogramHovered]     = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(116, 50, 77, 110);
        //style.Colors[ImGuiCol_DragDropTarget]           = 
        //style.Colors[ImGuiCol_NavHighlight]             = 
        //style.Colors[ImGuiCol_NavWindowingHighlight]    =
        //style.Colors[ImGuiCol_NavWindowingDimBg]        =
        style.Colors[ImGuiCol_ModalWindowDimBg]         = ImColor(51, 56, 69, 186);
    	break;
#pragma endregion
#pragma region LightGreen (WIP)
    case 22:
        if (applyStyle)
        {
            style.WindowRounding    = 2.f;
            style.ScrollbarRounding = 3.f;
            style.ScrollbarSize     = 16;
            style.FrameRounding     = style.ScrollbarRounding;
            style.FramePadding.x    = 6;
            style.FramePadding.y    = 4;
            style.GrabRounding      = style.WindowRounding;
            style.ChildRounding     = style.WindowRounding;
            style.IndentSpacing     = 22;
            style.ItemSpacing.x     = 10;
            style.ItemSpacing.y     = 4;
            style.AntiAliasedLines  = true;
            style.AntiAliasedFill   = true;
            style.Alpha             = 1.f;
        }

        style.Colors[ImGuiCol_Text]                     = ImColor(0, 0, 0, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(153, 153, 153, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(219, 219, 219, 255);
        style.Colors[ImGuiCol_ChildBg]                  = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(237, 237, 237, 250);
        style.Colors[ImGuiCol_Border]                   = ImColor(181, 181, 181, 20);
        style.Colors[ImGuiCol_BorderShadow]             = ImColor(0, 0, 0, 10);
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(181, 181, 181, 140);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(240, 240, 240, 140);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(181, 199, 176, 250);
        style.Colors[ImGuiCol_TitleBg]                  = ImColor(217, 217, 217, 255);
        style.Colors[ImGuiCol_TitleBgActive]            = ImColor(199, 199, 199, 255);
        style.Colors[ImGuiCol_TitleBgCollapsed]         = ImColor(209, 199, 199, 130);
        style.Colors[ImGuiCol_MenuBarBg]                = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_ScrollbarBg]              = ImColor(51, 64, 76, 156);
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(230, 230, 230, 76);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(235, 235, 235, 199);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = ImColor(255, 255, 255, 255);
        style.Colors[ImGuiCol_CheckMark]                = ImColor(47, 104, 49, 255);
        style.Colors[ImGuiCol_SliderGrab]               = ImColor(66, 150, 250, 199);
        style.Colors[ImGuiCol_SliderGrabActive]         = ImColor(66, 150, 250, 255);
        style.Colors[ImGuiCol_Button]                   = ImColor(181, 199, 176, 102);
        style.Colors[ImGuiCol_ButtonHovered]            = ImColor(185, 205, 179, 255);
        style.Colors[ImGuiCol_ButtonActive]             = ImColor(202, 230, 213, 255);
        style.Colors[ImGuiCol_Header]                   = ImColor(181, 199, 176, 79);
        style.Colors[ImGuiCol_HeaderHovered]            = ImColor(181, 199, 176, 204);
        style.Colors[ImGuiCol_HeaderActive]             = ImColor(181, 199, 176, 255);
        style.Colors[ImGuiCol_Separator]                = ImColor(99, 99, 99, 255);
        style.Colors[ImGuiCol_SeparatorHovered]         = ImColor(36, 112, 204, 199);
        style.Colors[ImGuiCol_SeparatorActive]          = ImColor(36, 112, 204, 255);
        style.Colors[ImGuiCol_ResizeGrip]               = ImColor(255, 255, 255, 0);
        style.Colors[ImGuiCol_ResizeGripHovered]        = ImColor(66, 150, 250, 115);
        style.Colors[ImGuiCol_ResizeGripActive]         = style.Colors[ImGuiCol_SliderGrab];
    	if (applyLerp)
        {
            style.Colors[ImGuiCol_Tab]                      = ImLerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 0.8f);
            style.Colors[ImGuiCol_TabHovered]               = style.Colors[ImGuiCol_HeaderHovered];
            style.Colors[ImGuiCol_TabActive]                = ImLerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.6f);
            style.Colors[ImGuiCol_TabUnfocused]             = ImLerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.8f);
            style.Colors[ImGuiCol_TabUnfocusedActive]       = ImLerp(style.Colors[ImGuiCol_TabActive], style.Colors[ImGuiCol_TitleBg], 0.4f);
        }
        style.Colors[ImGuiCol_PlotLines]                = style.Colors[ImGuiCol_Separator];
        style.Colors[ImGuiCol_PlotLinesHovered]         = ImColor(255, 110, 89, 255);
        style.Colors[ImGuiCol_PlotHistogram]            = ImColor(230, 178, 0, 255);
        style.Colors[ImGuiCol_PlotHistogramHovered]     = ImColor(255, 153, 0, 255);
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(66, 150, 250, 89);
        style.Colors[ImGuiCol_DragDropTarget]           = ImColor(66, 150, 250, 242);
        style.Colors[ImGuiCol_NavHighlight]             = style.Colors[ImGuiCol_HeaderHovered];
        style.Colors[ImGuiCol_NavWindowingHighlight]    = ImColor(178, 178, 178, 178);
        //style.Colors[ImGuiCol_NavWindowingDimBg]        =
        style.Colors[ImGuiCol_ModalWindowDimBg]         = ImColor(51, 51, 51, 89);
    	break;
#pragma endregion
#pragma region Photoshop (WIP)
    case 23:
        if (applyStyle)
        {
            style.FrameBorderSize   = 1.f;
            style.GrabMinSize       = 7.f;
            style.ScrollbarSize     = 13.f;
            style.TabBorderSize     = style.FrameBorderSize;
            style.ChildRounding     = 4.f;
            style.FrameRounding     = 2.f;
            style.ScrollbarRounding = 12.f;
            style.TabRounding       = 0.f;
            style.PopupRounding     = style.FrameRounding;
            style.WindowRounding    = style.ChildRounding;
        }

        style.Colors[ImGuiCol_Text]                     = ImColor(255, 255, 255, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(128, 128, 128, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(46, 46, 46, 255);
        style.Colors[ImGuiCol_ChildBg]                  = ImColor(71, 71, 71, 0);
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(80, 80, 80, 255);
        style.Colors[ImGuiCol_Border]                   = ImColor(68, 68, 68, 255);
        style.Colors[ImGuiCol_BorderShadow]             = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(41, 41, 41, 255);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(51, 51, 51, 255);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(71, 71, 71, 255);
        style.Colors[ImGuiCol_TitleBg]                  = ImColor(38, 38, 38, 255);
        style.Colors[ImGuiCol_TitleBgActive]            = style.Colors[ImGuiCol_TitleBg];
        style.Colors[ImGuiCol_TitleBgCollapsed]         = style.Colors[ImGuiCol_TitleBg];
        style.Colors[ImGuiCol_MenuBarBg]                = ImColor(50, 50, 50, 255);
        style.Colors[ImGuiCol_ScrollbarBg]              = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(71, 71, 71, 255);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(76, 76, 76, 255);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = ImColor(255, 100, 0, 255);
        style.Colors[ImGuiCol_CheckMark]                = style.Colors[ImGuiCol_Text];
        style.Colors[ImGuiCol_SliderGrab]               = ImColor(100, 100, 100, 255);
        style.Colors[ImGuiCol_SliderGrabActive]         = style.Colors[ImGuiCol_ScrollbarGrabActive];
        style.Colors[ImGuiCol_Button]                   = ImColor(255, 255, 255, 0);
        style.Colors[ImGuiCol_ButtonHovered]            = ImColor(255, 255, 255, 40);
        style.Colors[ImGuiCol_ButtonActive]             = ImColor(255, 255, 255, 100);
        style.Colors[ImGuiCol_Header]                   = style.Colors[ImGuiCol_PopupBg];
        style.Colors[ImGuiCol_HeaderHovered]            = ImColor(120, 120, 120, 255);
        style.Colors[ImGuiCol_HeaderActive]             = style.Colors[ImGuiCol_HeaderHovered];
        style.Colors[ImGuiCol_Separator]                = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_SeparatorHovered]         = style.Colors[ImGuiCol_SliderGrab];
        style.Colors[ImGuiCol_SeparatorActive]          = style.Colors[ImGuiCol_ScrollbarGrabActive];
        style.Colors[ImGuiCol_ResizeGrip]               = ImColor(255, 255, 255, 64);
        style.Colors[ImGuiCol_ResizeGripHovered]        = ImColor(255, 255, 255, 171);
        style.Colors[ImGuiCol_ResizeGripActive]         = style.Colors[ImGuiCol_ScrollbarGrabActive];
        style.Colors[ImGuiCol_Tab]                      = ImColor(25, 25, 25, 255);
        style.Colors[ImGuiCol_TabHovered]               = ImColor(90, 90, 90, 255);
        style.Colors[ImGuiCol_TabActive]                = style.Colors[ImGuiCol_MenuBarBg];
        style.Colors[ImGuiCol_TabUnfocused]             = style.Colors[ImGuiCol_Tab];
        style.Colors[ImGuiCol_TabUnfocusedActive]       = style.Colors[ImGuiCol_MenuBarBg];
        style.Colors[ImGuiCol_PlotLines]                = style.Colors[ImGuiCol_HeaderHovered];
        style.Colors[ImGuiCol_PlotLinesHovered]         = style.Colors[ImGuiCol_ScrollbarGrabActive];
        style.Colors[ImGuiCol_PlotHistogram]            = ImColor(149, 149, 149, 255);
        style.Colors[ImGuiCol_PlotHistogramHovered]     = style.Colors[ImGuiCol_ScrollbarGrabActive];
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = style.Colors[ImGuiCol_ButtonHovered];
        style.Colors[ImGuiCol_DragDropTarget]           = style.Colors[ImGuiCol_ScrollbarGrabActive];
        style.Colors[ImGuiCol_NavHighlight]             = style.Colors[ImGuiCol_ScrollbarGrabActive];
        style.Colors[ImGuiCol_NavWindowingHighlight]    = style.Colors[ImGuiCol_ScrollbarGrabActive];
        style.Colors[ImGuiCol_NavWindowingDimBg]        = ImColor(0, 0, 0, 149);
        style.Colors[ImGuiCol_ModalWindowDimBg]         = style.Colors[ImGuiCol_NavWindowingDimBg];
    	break;
#pragma endregion
#pragma region CorporateGrey (WIP)
    case 24:
        if (applyStyle)
        {
            style.ItemSpacing       = ImVec2(6, 2);
            style.WindowRounding    = 3;
            style.WindowPadding     = ImVec2(4, 4);
            style.WindowBorderSize  = 1;
            style.ChildRounding     = style.WindowRounding;
            style.ChildBorderSize   = style.WindowBorderSize;
            style.FrameRounding     = style.WindowRounding;
            style.FramePadding      = style.ItemSpacing;
            style.FrameBorderSize   = 0;
            style.ScrollbarRounding = 2;
            style.ScrollbarSize     = 18;
            style.PopupRounding     = style.WindowRounding;
            style.PopupBorderSize   = style.WindowBorderSize;
            style.GrabRounding      = style.WindowRounding;
            style.TabBorderSize     = style.FrameBorderSize;
            style.TabRounding       = style.WindowRounding;
        }

        style.Colors[ImGuiCol_Text]                     = ImColor(255, 255, 255, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(102, 102, 102, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(64, 64, 64, 255);
        style.Colors[ImGuiCol_ChildBg]                  = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_PopupBg]                  = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_Border]                   = ImColor(31, 31, 31, 181);
        style.Colors[ImGuiCol_BorderShadow]             = ImColor(255, 255, 255, 15);
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(107, 107, 107, 138);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(107, 107, 107, 102);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(143, 143, 143, 171);
        style.Colors[ImGuiCol_TitleBg]                  = ImColor(48, 48, 48, 255);
        style.Colors[ImGuiCol_TitleBgActive]            = ImColor(56, 56, 56, 255);
        style.Colors[ImGuiCol_TitleBgCollapsed]         = ImColor(43, 43, 43, 230);
        style.Colors[ImGuiCol_MenuBarBg]                = ImColor(85, 85, 85, 255);
        style.Colors[ImGuiCol_ScrollbarBg]              = ImColor(61, 61, 61, 135);
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(105, 105, 105, 255);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(133, 133, 133, 255);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = ImColor(194, 194, 194, 255);
        style.Colors[ImGuiCol_CheckMark]                = ImColor(166, 166, 166, 255);
        style.Colors[ImGuiCol_SliderGrab]               = style.Colors[ImGuiCol_ScrollbarGrabHovered];
        style.Colors[ImGuiCol_SliderGrabActive]         = ImColor(163, 163, 163, 255);
        style.Colors[ImGuiCol_Button]                   = ImColor(138, 138, 138, 89);
        style.Colors[ImGuiCol_ButtonHovered]            = ImColor(133, 133, 133, 150);
        style.Colors[ImGuiCol_ButtonActive]             = style.Colors[ImGuiCol_ScrollbarGrabActive];
        style.Colors[ImGuiCol_Header]                   = ImColor(97, 97, 97, 255);
        style.Colors[ImGuiCol_HeaderHovered]            = ImColor(120, 120, 120, 255);
        style.Colors[ImGuiCol_HeaderActive]             = ImColor(194, 194, 194, 196);
        style.Colors[ImGuiCol_Separator]                = ImColor(0, 0, 0, 35);
        style.Colors[ImGuiCol_SeparatorHovered]         = ImColor(178, 171, 153, 74);
        style.Colors[ImGuiCol_SeparatorActive]          = ImColor(179, 171, 153, 172);
        style.Colors[ImGuiCol_ResizeGrip]               = ImColor(66, 150, 250, 64);
        style.Colors[ImGuiCol_ResizeGripHovered]        = ImColor(66, 150, 250, 171);
        style.Colors[ImGuiCol_ResizeGripActive]         = ImColor(66, 150, 250, 242);
        style.Colors[ImGuiCol_Tab]                      = ImColor(64, 64, 64, 255);
        style.Colors[ImGuiCol_TabHovered]               = ImColor(102, 102, 102, 255);
        style.Colors[ImGuiCol_TabActive]                = ImColor(84, 84, 84, 255);
        style.Colors[ImGuiCol_TabUnfocused]             = style.Colors[ImGuiCol_Tab];
        style.Colors[ImGuiCol_TabUnfocusedActive]       = style.Colors[ImGuiCol_TabActive];
        style.Colors[ImGuiCol_PlotLines]                = ImColor(156, 156, 156, 255);
        style.Colors[ImGuiCol_PlotLinesHovered]         = ImColor(255, 110, 89, 255);
        style.Colors[ImGuiCol_PlotHistogram]            = ImColor(230, 178, 0, 255);
        style.Colors[ImGuiCol_PlotHistogramHovered]     = ImColor(255, 153, 0, 255);
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(186, 186, 186, 89);
        style.Colors[ImGuiCol_DragDropTarget]           = ImColor(255, 255, 0, 230);
        style.Colors[ImGuiCol_NavHighlight]             = ImColor(66, 150, 250, 255);
        style.Colors[ImGuiCol_NavWindowingHighlight]    = ImColor(255, 255, 255, 178);
        style.Colors[ImGuiCol_NavWindowingDimBg]        = ImColor(204, 204, 204, 51);
        style.Colors[ImGuiCol_ModalWindowDimBg]         = ImColor(204, 204, 204, 89);
    	break;
#pragma endregion
#pragma region Violet (WIP)
    case 25:
        if (applyStyle)
            style.WindowMenuButtonPosition = ImGuiDir_Right;
    	
        style.Colors[ImGuiCol_Text]                     = ImColor(244, 241, 222, 204);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(91, 88, 69, 204);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(37, 33, 49, 204);
        style.Colors[ImGuiCol_ChildBg]                  = ImColor(0, 0, 0, 51);
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(37, 33, 49, 230);
        style.Colors[ImGuiCol_Border]                   = ImColor(139, 135, 151, 204);
        style.Colors[ImGuiCol_BorderShadow]             = ImColor(0, 0, 0, 204);
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(121, 35, 89, 102);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(172, 86, 140, 102);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(147, 61, 115, 255);
        style.Colors[ImGuiCol_TitleBg]                  = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_TitleBgActive]            = ImColor(62, 59, 75, 255);
        style.Colors[ImGuiCol_TitleBgCollapsed]         = style.Colors[ImGuiCol_BorderShadow];
        style.Colors[ImGuiCol_MenuBarBg]                = style.Colors[ImGuiCol_BorderShadow];
        style.Colors[ImGuiCol_ScrollbarBg]              = ImColor(139, 135, 151, 128);
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(113, 110, 126, 204);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(164, 161, 177, 204);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = ImColor(139, 135, 151, 255);
        style.Colors[ImGuiCol_CheckMark]                = ImColor(199, 239, 0, 204);
        style.Colors[ImGuiCol_SliderGrab]               = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_SliderGrabActive]         = ImColor(224, 264, 26, 255);
        style.Colors[ImGuiCol_Button]                   = ImColor(218, 17, 94, 204);
        style.Colors[ImGuiCol_ButtonHovered]            = ImColor(269, 68, 145, 204);
        style.Colors[ImGuiCol_ButtonActive]             = ImColor(244, 43, 119, 255);
        style.Colors[ImGuiCol_Header]                   = ImColor(121, 35, 89, 204);
        style.Colors[ImGuiCol_HeaderHovered]            = ImColor(172, 86, 140, 204);
        style.Colors[ImGuiCol_HeaderActive]             = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Separator]                = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_SeparatorHovered]         = ImColor(190, 186, 202, 204);
        style.Colors[ImGuiCol_SeparatorActive]          = ImColor(164, 161, 177, 255);
        style.Colors[ImGuiCol_ResizeGrip]               = ImColor(218, 17, 94, 51);
        style.Colors[ImGuiCol_ResizeGripHovered]        = ImColor(269, 68, 145, 51);
        style.Colors[ImGuiCol_ResizeGripActive]         = style.Colors[ImGuiCol_ButtonActive];
        style.Colors[ImGuiCol_Tab]                      = ImColor(218, 17, 94, 153);
        style.Colors[ImGuiCol_TabHovered]               = ImColor(269, 68, 145, 153);
        style.Colors[ImGuiCol_TabActive]                = style.Colors[ImGuiCol_ButtonActive];
        style.Colors[ImGuiCol_TabUnfocused]             = style.Colors[ImGuiCol_Tab];
        style.Colors[ImGuiCol_TabUnfocusedActive]       = style.Colors[ImGuiCol_TabActive];
        style.Colors[ImGuiCol_PlotLines]                = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_PlotLinesHovered]         = ImColor(250, 290, 51, 204);
        style.Colors[ImGuiCol_PlotHistogram]            = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_PlotHistogramHovered]     = style.Colors[ImGuiCol_PlotLinesHovered];
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(199, 239, 0, 102);
        style.Colors[ImGuiCol_DragDropTarget]           = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_NavHighlight]             = ImColor(255, 255, 255, 204);
        style.Colors[ImGuiCol_NavWindowingHighlight]    = style.Colors[ImGuiCol_NavHighlight];
        style.Colors[ImGuiCol_NavWindowingDimBg]        = ImColor(255, 255, 255, 51);
        style.Colors[ImGuiCol_ModalWindowDimBg]         = ImColor(0, 0, 0, 153);
		break;
#pragma endregion
#pragma region Raikiri (WIP)
    case 26:
        if (applyStyle)
        {
            style.FrameRounding = 4.f;
            style.GrabRounding  = style.FrameRounding;
        }

        style.Colors[ImGuiCol_Text]                     = ImColor(242, 245, 250, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(92, 107, 120, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(28, 38, 43, 255);
        style.Colors[ImGuiCol_ChildBg]                  = ImColor(38, 46, 56, 255);
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(20, 20, 20, 240);
        style.Colors[ImGuiCol_Border]                   = ImColor(20, 26, 31, 255);
        style.Colors[ImGuiCol_BorderShadow]             = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(51, 64, 74, 255);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(31, 51, 71, 255);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(23, 31, 36, 255);
        style.Colors[ImGuiCol_TitleBg]                  = ImColor(23, 31, 36, 166);
        style.Colors[ImGuiCol_TitleBgActive]            = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_TitleBgCollapsed]         = ImColor(0, 0, 0, 130);
        style.Colors[ImGuiCol_MenuBarBg]                = style.Colors[ImGuiCol_ChildBg];
        style.Colors[ImGuiCol_ScrollbarBg]              = ImColor(5, 5, 5, 99);
        style.Colors[ImGuiCol_ScrollbarGrab]            = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(46, 56, 64, 255);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = ImColor(23, 54, 79, 255);
        style.Colors[ImGuiCol_CheckMark]                = ImColor(71, 143, 255, 255);
        style.Colors[ImGuiCol_SliderGrab]               = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_SliderGrabActive]         = ImColor(94, 156, 255, 255);
        style.Colors[ImGuiCol_Button]                   = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ButtonHovered]            = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_ButtonActive]             = ImColor(15, 135, 250, 255);
        style.Colors[ImGuiCol_Header]                   = ImColor(51, 64, 74, 140);
        style.Colors[ImGuiCol_HeaderHovered]            = ImColor(66, 150, 250, 204);
        style.Colors[ImGuiCol_HeaderActive]             = ImColor(66, 150, 250, 255);
        style.Colors[ImGuiCol_Separator]                = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_SeparatorHovered]         = ImColor(26, 102, 191, 199);
        style.Colors[ImGuiCol_SeparatorActive]          = ImColor(26, 102, 191, 255);
        style.Colors[ImGuiCol_ResizeGrip]               = ImColor(66, 150, 250, 64);
        style.Colors[ImGuiCol_ResizeGripHovered]        = ImColor(66, 150, 250, 171);
        style.Colors[ImGuiCol_ResizeGripActive]         = ImColor(66, 150, 250, 242);
        style.Colors[ImGuiCol_Tab]                      = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_TabHovered]               = style.Colors[ImGuiCol_HeaderHovered];
        style.Colors[ImGuiCol_TabActive]                = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_TabUnfocused]             = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_TabUnfocusedActive]       = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_PlotLines]                = ImColor(156, 156, 156, 255);
        style.Colors[ImGuiCol_PlotLinesHovered]         = ImColor(255, 110, 89, 255);
        style.Colors[ImGuiCol_PlotHistogram]            = ImColor(230, 178, 0, 255);
        style.Colors[ImGuiCol_PlotHistogramHovered]     = ImColor(255, 153, 0, 255);
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(66, 150, 250, 89);
        style.Colors[ImGuiCol_DragDropTarget]           = ImColor(255, 255, 0, 230);
        style.Colors[ImGuiCol_NavHighlight]             = style.Colors[ImGuiCol_HeaderActive];
        style.Colors[ImGuiCol_NavWindowingHighlight]    = ImColor(255, 255, 255, 178);
        style.Colors[ImGuiCol_NavWindowingDimBg]        = ImColor(204, 204, 204, 51);
        style.Colors[ImGuiCol_ModalWindowDimBg]         = ImColor(204, 204, 204, 89);
    	break;
#pragma endregion
#pragma region VGUI (Valve GUI) (WIP)
    case 27:
        if (applyStyle)
        {
            style.FrameRounding     = 0.f;
            style.FrameBorderSize   = 1.f;
            style.WindowRounding    = style.FrameRounding;
            style.ChildRounding     = style.FrameRounding;
            style.PopupRounding     = style.FrameRounding;
            style.ScrollbarRounding = style.FrameRounding;
            style.GrabRounding      = style.FrameRounding;
            style.TabRounding       = style.FrameRounding;
        }

        style.Colors[ImGuiCol_Text]                     = ImColor(255, 255, 255, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(128, 128, 128, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(74, 87, 66, 255);
        style.Colors[ImGuiCol_ChildBg]                  = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(61, 69, 51, 255);
        style.Colors[ImGuiCol_Border]                   = ImColor(138, 145, 130, 128);
        style.Colors[ImGuiCol_BorderShadow]             = ImColor(36, 41, 28, 133);
        style.Colors[ImGuiCol_FrameBg]                  = style.Colors[ImGuiCol_PopupBg];
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(69, 76, 59, 255);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(76, 87, 66, 255);
        style.Colors[ImGuiCol_TitleBg]                  = style.Colors[ImGuiCol_PopupBg];
        style.Colors[ImGuiCol_TitleBgActive]            = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_TitleBgCollapsed]         = ImColor(0, 0, 0, 130);
        style.Colors[ImGuiCol_MenuBarBg]                = style.Colors[ImGuiCol_PopupBg];
        style.Colors[ImGuiCol_ScrollbarBg]              = ImColor(89, 107, 79, 255);
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(71, 82, 61, 255);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(64, 76, 56, 255);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = ImColor(59, 69, 54, 255);
        style.Colors[ImGuiCol_CheckMark]                = ImColor(150, 138, 46, 255);
        style.Colors[ImGuiCol_SliderGrab]               = style.Colors[ImGuiCol_ScrollbarBg];
        style.Colors[ImGuiCol_SliderGrabActive]         = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_Button]                   = ImColor(74, 87, 66, 102);
        style.Colors[ImGuiCol_ButtonHovered]            = style.Colors[ImGuiCol_ScrollbarBg];
        style.Colors[ImGuiCol_ButtonActive]             = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_Header]                   = style.Colors[ImGuiCol_ScrollbarBg];
        style.Colors[ImGuiCol_HeaderHovered]            = ImColor(89, 107, 79, 153);
        style.Colors[ImGuiCol_HeaderActive]             = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_Separator]                = ImColor(36, 41, 28, 255);
        style.Colors[ImGuiCol_SeparatorHovered]         = ImColor(138, 145, 130, 255);
        style.Colors[ImGuiCol_SeparatorActive]          = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_ResizeGrip]               = ImColor(48, 59, 46, 0);
        style.Colors[ImGuiCol_ResizeGripHovered]        = ImColor(138, 145, 130, 255);
        style.Colors[ImGuiCol_ResizeGripActive]         = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_Tab]                      = style.Colors[ImGuiCol_ScrollbarBg];
        style.Colors[ImGuiCol_TabHovered]               = ImColor(138, 145, 130, 199);
        style.Colors[ImGuiCol_TabActive]                = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_TabUnfocused]             = style.Colors[ImGuiCol_PopupBg];
        style.Colors[ImGuiCol_TabUnfocusedActive]       = style.Colors[ImGuiCol_ScrollbarBg];
        style.Colors[ImGuiCol_PlotLines]                = ImColor(156, 156, 156, 255);
        style.Colors[ImGuiCol_PlotLinesHovered]         = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_PlotHistogram]            = ImColor(255, 199, 71, 255);
        style.Colors[ImGuiCol_PlotHistogramHovered]     = ImColor(255, 153, 0, 255);
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_DragDropTarget]           = ImColor(186, 171, 61, 255);
        style.Colors[ImGuiCol_NavHighlight]             = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_NavWindowingHighlight]    = ImColor(255, 255, 255, 178);
        style.Colors[ImGuiCol_NavWindowingDimBg]        = ImColor(204, 204, 204, 51);
        style.Colors[ImGuiCol_ModalWindowDimBg]         = ImColor(204, 204, 204, 89);
    	break;
#pragma endregion
#pragma region Gold & Black (WIP)
    case 28:
        if (applyStyle)
        {
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
        }

        style.Colors[ImGuiCol_Text]                     = ImColor(235, 235, 235, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(112, 112, 112, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(15, 15, 15, 255);
        style.Colors[ImGuiCol_ChildBg]                  = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(20, 20, 20, 240);
        style.Colors[ImGuiCol_Border]                   = ImColor(130, 92, 38, 255);
        style.Colors[ImGuiCol_BorderShadow]             = style.Colors[ImGuiCol_ChildBg];
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(28, 28, 28, 255);
        style.Colors[ImGuiCol_FrameBgHovered]           = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(199, 140, 54, 255);
        style.Colors[ImGuiCol_TitleBg]                  = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_TitleBgActive]            = ImColor(232, 163, 33, 255);
        style.Colors[ImGuiCol_TitleBgCollapsed]         = ImColor(0, 0, 0, 130);
        style.Colors[ImGuiCol_MenuBarBg]                = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ScrollbarBg]              = ImColor(15, 15, 15, 135);
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(54, 54, 54, 255);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(120, 120, 120, 255);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = ImColor(207, 212, 207, 255);
        style.Colors[ImGuiCol_CheckMark]                = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_SliderGrab]               = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_SliderGrabActive]         = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_Button]                   = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_ButtonHovered]            = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_ButtonActive]             = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Header]                   = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_HeaderHovered]            = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_HeaderActive]             = ImColor(237, 166, 36, 255);
        style.Colors[ImGuiCol_Separator]                = style.Colors[ImGuiCol_ScrollbarGrab];
        style.Colors[ImGuiCol_SeparatorHovered]         = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_SeparatorActive]          = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_ResizeGrip]               = style.Colors[ImGuiCol_ScrollbarGrab];
        style.Colors[ImGuiCol_ResizeGripHovered]        = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_ResizeGripActive]         = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Tab]                      = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_TabHovered]               = style.Colors[ImGuiCol_TitleBgActive];
        style.Colors[ImGuiCol_TabActive]                = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_TabUnfocused]             = ImColor(18, 26, 38, 247);
        style.Colors[ImGuiCol_TabUnfocusedActive]       = ImColor(36, 66, 107, 255);
        style.Colors[ImGuiCol_PlotLines]                = ImColor(156, 156, 156, 255);
        style.Colors[ImGuiCol_PlotLinesHovered]         = ImColor(255, 110, 89, 255);
        style.Colors[ImGuiCol_PlotHistogram]            = ImColor(230, 178, 0, 255);
        style.Colors[ImGuiCol_PlotHistogramHovered]     = ImColor(255, 153, 0, 255);
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(66, 150, 250, 89);
        style.Colors[ImGuiCol_DragDropTarget]           = ImColor(255, 255, 0, 230);
        style.Colors[ImGuiCol_NavHighlight]             = ImColor(66, 150, 250, 255);
        style.Colors[ImGuiCol_NavWindowingHighlight]    = ImColor(255, 255, 255, 178);
        style.Colors[ImGuiCol_NavWindowingDimBg]        = ImColor(204, 204, 204, 51);
        style.Colors[ImGuiCol_ModalWindowDimBg]         = ImColor(204, 204, 204, 89);
    	break;
#pragma endregion
#pragma region Sewer (WIP)
    case 29:
        if (applyStyle)
        {
            style.FrameRounding     = 4.f;
            style.GrabRounding      = style.FrameRounding;
            style.WindowBorderSize  = 0.f;
            style.PopupBorderSize   = style.WindowBorderSize;
        }

        style.Colors[ImGuiCol_Text]                     = ImColor(255, 255, 255, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(186, 191, 189, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(23, 23, 23, 240);
        style.Colors[ImGuiCol_ChildBg]                  = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(20, 20, 20, 240);
        style.Colors[ImGuiCol_Border]                   = ImColor(51, 51, 51, 128);
        style.Colors[ImGuiCol_BorderShadow]             = style.Colors[ImGuiCol_ChildBg];
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(181, 99, 99, 138);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(214, 168, 168, 102);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(214, 168, 168, 171);
        style.Colors[ImGuiCol_TitleBg]                  = ImColor(120, 56, 56, 171);
        style.Colors[ImGuiCol_TitleBgActive]            = ImColor(120, 56, 56, 255);
        style.Colors[ImGuiCol_TitleBgCollapsed]         = style.Colors[ImGuiCol_TitleBg];
        style.Colors[ImGuiCol_MenuBarBg]                = ImColor(87, 41, 41, 255);
        style.Colors[ImGuiCol_ScrollbarBg]              = ImColor(5, 5, 5, 135);
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(79, 79, 79, 255);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(105, 105, 105, 255);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = ImColor(130, 130, 130, 255);
        style.Colors[ImGuiCol_CheckMark]                = style.Colors[ImGuiCol_Text];
        style.Colors[ImGuiCol_SliderGrab]               = ImColor(181, 99, 99, 255);
        style.Colors[ImGuiCol_SliderGrabActive]         = ImColor(214, 168, 168, 255);
        style.Colors[ImGuiCol_Button]                   = ImColor(120, 56, 56, 166);
        style.Colors[ImGuiCol_ButtonHovered]            = ImColor(181, 99, 99, 166);
        style.Colors[ImGuiCol_ButtonActive]             = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_Header]                   = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_HeaderHovered]            = ImColor(214, 168, 168, 166);
        style.Colors[ImGuiCol_HeaderActive]             = ImColor(214, 168, 168, 0);
        style.Colors[ImGuiCol_Separator]                = ImColor(110, 110, 128, 128);
        style.Colors[ImGuiCol_SeparatorHovered]         = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_SeparatorActive]          = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ResizeGrip]               = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ResizeGripHovered]        = ImColor(214, 168, 168, 168);
        style.Colors[ImGuiCol_ResizeGripActive]         = style.Colors[ImGuiCol_ResizeGripHovered];
        style.Colors[ImGuiCol_Tab]                      = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_TabHovered]               = style.Colors[ImGuiCol_ResizeGripHovered];
        style.Colors[ImGuiCol_TabActive]                = style.Colors[ImGuiCol_ResizeGripHovered];
        style.Colors[ImGuiCol_TabUnfocused]             = ImColor(18, 26, 38, 247);
        style.Colors[ImGuiCol_TabUnfocusedActive]       = ImColor(36, 66, 107, 255);
        style.Colors[ImGuiCol_PlotLines]                = ImColor(156, 156, 156, 255);
        style.Colors[ImGuiCol_PlotLinesHovered]         = ImColor(255, 110, 89, 255);
        style.Colors[ImGuiCol_PlotHistogram]            = ImColor(230, 178, 0, 255);
        style.Colors[ImGuiCol_PlotHistogramHovered]     = ImColor(255, 153, 0, 255);
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(66, 150, 250, 89);
        style.Colors[ImGuiCol_DragDropTarget]           = ImColor(255, 255, 0, 230);
        style.Colors[ImGuiCol_NavHighlight]             = style.Colors[ImGuiCol_ScrollbarGrabHovered];
        style.Colors[ImGuiCol_NavWindowingHighlight]    = ImColor(255, 255, 255, 178);
        style.Colors[ImGuiCol_NavWindowingDimBg]        = ImColor(204, 204, 204, 51);
        style.Colors[ImGuiCol_ModalWindowDimBg]         = ImColor(204, 204, 204, 89);
    	break;
#pragma endregion
#pragma region VS (Visual Studio) (WIP)
    case 30:
        if (applyStyle)
        {
            style.WindowRounding    = 0.f;
            style.ChildRounding     = style.WindowRounding;
            style.FrameRounding     = style.WindowRounding;
            style.GrabRounding      = style.WindowRounding;
            style.PopupRounding     = style.WindowRounding;
            style.ScrollbarRounding = style.WindowRounding;
            style.TabRounding       = style.WindowRounding;
        }

        style.Colors[ImGuiCol_Text]                     = ImColor(255, 255, 255, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(151, 151, 151, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(37, 37, 38, 255);
        style.Colors[ImGuiCol_ChildBg]                  = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_PopupBg]                  = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_Border]                   = ImColor(78, 78, 78, 255);
        style.Colors[ImGuiCol_BorderShadow]             = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(51, 51, 55, 255);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(29, 151, 236, 255);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(0, 119, 200, 255);
        style.Colors[ImGuiCol_TitleBg]                  = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_TitleBgActive]            = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_TitleBgCollapsed]         = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_MenuBarBg]                = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ScrollbarBg]              = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(82, 82, 85, 255);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(90, 90, 95, 255);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = style.Colors[ImGuiCol_ScrollbarGrabHovered];
        style.Colors[ImGuiCol_CheckMark]                = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_SliderGrab]               = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_SliderGrabActive]         = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Button]                   = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ButtonHovered]            = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_ButtonActive]             = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_Header]                   = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_HeaderHovered]            = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_HeaderActive]             = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Separator]                = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_SeparatorHovered]         = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_SeparatorActive]          = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_ResizeGrip]               = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_ResizeGripHovered]        = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ResizeGripActive]         = style.Colors[ImGuiCol_ScrollbarGrab];
        style.Colors[ImGuiCol_Tab]                      = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_TabHovered]               = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_TabActive]                = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_TabUnfocused]             = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_TabUnfocusedActive]       = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_PlotLines]                = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_PlotLinesHovered]         = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_PlotHistogram]            = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_PlotHistogramHovered]     = style.Colors[ImGuiCol_FrameBgHovered];
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_DragDropTarget]           = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_NavHighlight]             = style.Colors[ImGuiCol_WindowBg];
        //style.Colors[ImGuiCol_NavWindowingHighlight]    = 
        //style.Colors[ImGuiCol_NavWindowingDimBg]        = 
        style.Colors[ImGuiCol_ModalWindowDimBg]         = style.Colors[ImGuiCol_WindowBg];
    	break;
#pragma endregion
#pragma region OverShifted (WIP)
    case 31:
        if (applyStyle)
        {
            style.GrabRounding  = 2.3f;
            style.FrameRounding = style.GrabRounding;
        }

        style.Colors[ImGuiCol_Text]                     = ImColor(255, 255, 255, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(128, 128, 128, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(33, 36, 38, 255);
        style.Colors[ImGuiCol_ChildBg]                  = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_PopupBg]                  = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_Border]                   = ImColor(110, 110, 128, 128);
        style.Colors[ImGuiCol_BorderShadow]             = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(64, 64, 64, 255);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(97, 97, 97, 255);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(171, 171, 171, 99);
        style.Colors[ImGuiCol_TitleBg]                  = ImColor(20, 20, 23, 255);
        style.Colors[ImGuiCol_TitleBgActive]            = style.Colors[ImGuiCol_TitleBg];
        style.Colors[ImGuiCol_TitleBgCollapsed]         = ImColor(0, 0, 0, 130);
        style.Colors[ImGuiCol_MenuBarBg]                = ImColor(36, 36, 36, 255);
        style.Colors[ImGuiCol_ScrollbarBg]              = ImColor(5, 5, 5, 135);
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(79, 79, 79, 255);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(105, 105, 105, 255);
        style.Colors[ImGuiCol_ScrollbarGrabActive]      = ImColor(130, 130, 130, 255);
        style.Colors[ImGuiCol_CheckMark]                = ImColor(28, 163, 235, 255);
        style.Colors[ImGuiCol_SliderGrab]               = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_SliderGrabActive]         = ImColor(20, 128, 184, 255);
        style.Colors[ImGuiCol_Button]                   = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_ButtonHovered]            = style.Colors[ImGuiCol_FrameBgHovered];
        style.Colors[ImGuiCol_ButtonActive]             = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Header]                   = ImColor(56, 56, 56, 255);
        style.Colors[ImGuiCol_HeaderHovered]            = style.Colors[ImGuiCol_FrameBg];
        style.Colors[ImGuiCol_HeaderActive]             = style.Colors[ImGuiCol_FrameBgActive];
        style.Colors[ImGuiCol_Separator]                = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_SeparatorHovered]         = ImColor(105, 107, 112, 255);
        style.Colors[ImGuiCol_SeparatorActive]          = ImColor(66, 150, 250, 242);
        style.Colors[ImGuiCol_ResizeGrip]               = style.Colors[ImGuiCol_BorderShadow];
        style.Colors[ImGuiCol_ResizeGripHovered]        = ImColor(74, 76, 79, 171);
        style.Colors[ImGuiCol_ResizeGripActive]         = style.Colors[ImGuiCol_SeparatorActive];
        style.Colors[ImGuiCol_Tab]                      = ImColor(20, 20, 23, 212);
        style.Colors[ImGuiCol_TabHovered]               = ImColor(84, 87, 92, 212);
        style.Colors[ImGuiCol_TabActive]                = ImColor(59, 59, 61, 255);
        style.Colors[ImGuiCol_TabUnfocused]             = style.Colors[ImGuiCol_TitleBg];
        style.Colors[ImGuiCol_TabUnfocusedActive]       = style.Colors[ImGuiCol_WindowBg];
        style.Colors[ImGuiCol_PlotLines]                = ImColor(156, 156, 156, 255);
        style.Colors[ImGuiCol_PlotLinesHovered]         = ImColor(255, 110, 89, 255);
        style.Colors[ImGuiCol_PlotHistogram]            = ImColor(230, 178, 0, 255);
        style.Colors[ImGuiCol_PlotHistogramHovered]     = ImColor(255, 153, 0, 255);
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(66, 150, 250, 89);
        style.Colors[ImGuiCol_DragDropTarget]           = style.Colors[ImGuiCol_CheckMark];
        style.Colors[ImGuiCol_NavHighlight]             = ImColor(66, 150, 250, 255);
        style.Colors[ImGuiCol_NavWindowingHighlight]    = ImColor(255, 255, 255, 178);
        style.Colors[ImGuiCol_NavWindowingDimBg]        = ImColor(204, 204, 204, 51);
        style.Colors[ImGuiCol_ModalWindowDimBg]         = ImColor(204, 204, 204, 89);
    	break;
#pragma endregion
#pragma region RayTeak (WIP)
    case 32:
        if (applyStyle)
        {
            style.WindowPadding     = ImVec2(15, 15);
            style.WindowRounding    = 5.f;
            style.FramePadding      = ImVec2(5, 5);
            style.FrameRounding     = 4.f;
            style.ItemSpacing       = ImVec2(12, 8);
            style.ItemInnerSpacing  = ImVec2(8, 6);
            style.IndentSpacing     = 25.f;
            style.ScrollbarSize     = 15.f;
            style.ScrollbarRounding = 9.f;
            style.GrabMinSize       = 5.f;
            style.GrabRounding      = 3.f;
        }

        style.Colors[ImGuiCol_Text]                     = ImColor(204, 204, 212, 255);
        style.Colors[ImGuiCol_TextDisabled]             = ImColor(61, 59, 74, 255);
        style.Colors[ImGuiCol_WindowBg]                 = ImColor(15, 13, 18, 255);
        style.Colors[ImGuiCol_ChildBg]                  = ImColor(18, 18, 23, 255);
        style.Colors[ImGuiCol_PopupBg]                  = ImColor(18, 18, 23, 255);
        style.Colors[ImGuiCol_Border]                   = ImColor(204, 204, 212, 224);
        style.Colors[ImGuiCol_BorderShadow]             = ImColor(235, 232, 224, 0);
        style.Colors[ImGuiCol_FrameBg]                  = ImColor(26, 23, 31, 255);
        style.Colors[ImGuiCol_FrameBgHovered]           = ImColor(61, 59, 74, 255);
        style.Colors[ImGuiCol_FrameBgActive]            = ImColor(143, 143, 148, 255);
        style.Colors[ImGuiCol_TitleBg]                  = ImColor(26, 23, 31, 255);
        style.Colors[ImGuiCol_TitleBgActive]            = ImColor(18, 18, 23, 255);
        style.Colors[ImGuiCol_TitleBgCollapsed]         = ImColor(255, 250, 242, 191);
        style.Colors[ImGuiCol_MenuBarBg]                = ImColor(26, 23, 31, 255);
        style.Colors[ImGuiCol_ScrollbarBg]              = ImColor(26, 23, 31, 255);
        style.Colors[ImGuiCol_ScrollbarGrab]            = ImColor(204, 204, 212, 79);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImColor(143, 143, 148, 255);
        //style.Colors[ImGuiCol_ScrollbarGrabActive]      = 
        style.Colors[ImGuiCol_CheckMark]                = ImColor(204, 204, 212, 79);
        style.Colors[ImGuiCol_SliderGrab]               = ImColor(204, 204, 212, 79);
        style.Colors[ImGuiCol_SliderGrabActive]         = ImColor(15, 13, 18, 255);
        style.Colors[ImGuiCol_Button]                   = ImColor(26, 23, 31, 255);
        style.Colors[ImGuiCol_ButtonHovered]            = ImColor(61, 59, 74, 255);
        style.Colors[ImGuiCol_ButtonActive]             = ImColor(143, 143, 148, 255);
        style.Colors[ImGuiCol_Header]                   = ImColor(26, 23, 31, 255);
        style.Colors[ImGuiCol_HeaderHovered]            = ImColor(143, 143, 148, 255);
        style.Colors[ImGuiCol_HeaderActive]             = ImColor(15, 13, 18, 255);
        style.Colors[ImGuiCol_Separator]                = ImColor(143, 143, 148, 255);
        style.Colors[ImGuiCol_SeparatorHovered]         = ImColor(61, 59, 74, 255);
        style.Colors[ImGuiCol_SeparatorActive]          = ImColor(143, 143, 148, 255);
        style.Colors[ImGuiCol_ResizeGrip]               = ImColor(0, 0, 0, 0);
        style.Colors[ImGuiCol_ResizeGripHovered]        = ImColor(143, 143, 148, 255);
        style.Colors[ImGuiCol_ResizeGripActive]         = ImColor(15, 13, 18, 255);
    	if (applyLerp)
    	{
            style.Colors[ImGuiCol_Tab]                      = ImLerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 0.8f);
            style.Colors[ImGuiCol_TabHovered]               = style.Colors[ImGuiCol_HeaderHovered];
            style.Colors[ImGuiCol_TabActive]                = ImLerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.6f);
            style.Colors[ImGuiCol_TabUnfocused]             = ImLerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.8f);
            style.Colors[ImGuiCol_TabUnfocusedActive]       = ImLerp(style.Colors[ImGuiCol_TabActive], style.Colors[ImGuiCol_TitleBg], 0.4f);
    	}
        style.Colors[ImGuiCol_PlotLines]                = ImColor(102, 99, 97, 161);
        style.Colors[ImGuiCol_PlotLinesHovered]         = ImColor(64, 255, 0, 255);
        style.Colors[ImGuiCol_PlotHistogram]            = ImColor(102, 99, 97, 161);
        style.Colors[ImGuiCol_PlotHistogramHovered]     = ImColor(64, 255, 0, 255);
        //style.Colors[ImGuiCol_TableHeaderBg]            = 
        //style.Colors[ImGuiCol_TableBorderStrong]        = 
        //style.Colors[ImGuiCol_TableBorderLight]         = 
        //style.Colors[ImGuiCol_TableRowBg]               = 
        //style.Colors[ImGuiCol_TableRowBgAlt]            = 
        style.Colors[ImGuiCol_TextSelectedBg]           = ImColor(64, 255, 0, 110);
        //style.Colors[ImGuiCol_DragDropTarget]           = 
        //style.Colors[ImGuiCol_NavHighlight]             = 
        //style.Colors[ImGuiCol_NavWindowingHighlight]    = 
        //style.Colors[ImGuiCol_NavWindowingDimBg]        = 
        style.Colors[ImGuiCol_ModalWindowDimBg]         = ImColor(255, 250, 242, 186);
    	break;
#pragma endregion
#pragma region Aiekick (Green / Blue)
    case 33:
        style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.44f, 0.44f, 0.44f, 0.60f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.57f, 0.57f, 0.57f, 0.70f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.76f, 0.76f, 0.76f, 0.80f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.13f, 0.75f, 0.55f, 0.80f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.13f, 0.75f, 0.75f, 0.80f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
        style.Colors[ImGuiCol_Tab] = ImVec4(0.13f, 0.75f, 0.55f, 0.80f);
        style.Colors[ImGuiCol_TabHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.80f);
        style.Colors[ImGuiCol_TabActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
        style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
        style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.36f, 0.36f, 0.36f, 0.54f);
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
        style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
        style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
        style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
        break;
#pragma endregion
#pragma region Aiekick (Red/Dark)
    case 34:
        style.Colors[ImGuiCol_Text] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.94f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.37f, 0.14f, 0.14f, 0.67f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.39f, 0.20f, 0.20f, 0.67f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.48f, 0.16f, 0.16f, 1.00f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.48f, 0.16f, 0.16f, 1.00f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.56f, 0.10f, 0.10f, 1.00f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 0.19f, 0.19f, 0.40f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.89f, 0.00f, 0.19f, 1.00f);
        style.Colors[ImGuiCol_Button] = ImVec4(1.00f, 0.19f, 0.19f, 0.40f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.80f, 0.17f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.89f, 0.00f, 0.19f, 1.00f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.33f, 0.35f, 0.36f, 0.53f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.76f, 0.28f, 0.44f, 0.67f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.47f, 0.47f, 0.47f, 0.67f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.85f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
        style.Colors[ImGuiCol_Tab] = ImVec4(0.07f, 0.07f, 0.07f, 0.51f);
        style.Colors[ImGuiCol_TabHovered] = ImVec4(0.86f, 0.23f, 0.43f, 0.67f);
        style.Colors[ImGuiCol_TabActive] = ImVec4(0.19f, 0.19f, 0.19f, 0.57f);
        style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.05f, 0.05f, 0.05f, 0.90f);
        style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.13f, 0.13f, 0.74f);
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
        style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
        style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
        style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
        break;
#pragma endregion
    default: break;
    }
}

json Style::toJSON() noexcept
{
    json j;

    // Save window size
    to_json(j["Window Size"], ImGui::GetStyle().WindowMinSize, ImVec2(600.f, 350.f));
	
	// Save styleColors struct
    {
        const auto& o = styleConfig;
        const decltype(styleConfig) dummy;

        if (o.colors != dummy.colors)
            if (o.colors[0])
                j["Current Color"] = o.colors;
        // Apply Style
        {
            auto& applyStyleJson = j["Apply Style"];
            for (std::size_t i = 0, e = o.applyStyle.size(); i != e; ++i)
                if (o.applyStyle.at(i) != dummy.applyStyle.at(i))
                    applyStyleJson[avaliableColors.at(i)] = o.applyStyle.at(i);
        }
        // Apply Lerp
        {
            auto& applyLerpJson = j["Apply Lerp"];
            for (std::size_t i = 0, e = o.applyLerp.size(); i != e; ++i)
                if (o.applyLerp.at(i) != dummy.applyLerp.at(i))
                    applyLerpJson[avaliableColors.at(i)] = o.applyLerp.at(i);
        }
        if (o.fonts != dummy.fonts)
            if (o.fonts[0])
                j["Current Font"] = o.fonts;

        WRITE("Alpha Flags", alpha_flags)

        WRITE_OBJ("Background Color", backgroundColor);
        WRITE_OBJ("Text Color", textColor);
        WRITE_OBJ("Main Color", mainColor);
        WRITE_OBJ("Main Accent Color", mainAccentColor);
        WRITE_OBJ("Highlight Color", highlightColor);
    }

	// Save 'Sizes' tab
    {
        const auto& o = ImGui::GetStyle();
        const ImGuiStyle dummy{};
        auto& sizesJson = j["Sizes"];
    	
#define WRITE_STYLE(valueName) if (!(o.valueName == dummy.valueName)) sizesJson[#valueName] = o.valueName
#define WRITE_STYLE_OBJ(valueName) to_json(sizesJson[#valueName], o.valueName, dummy.valueName)
    	
        // Main
        WRITE_STYLE_OBJ(WindowPadding);
        WRITE_STYLE_OBJ(FramePadding);
        WRITE_STYLE_OBJ(CellPadding);
        WRITE_STYLE_OBJ(ItemSpacing);
        WRITE_STYLE_OBJ(ItemInnerSpacing);
        WRITE_STYLE_OBJ(TouchExtraPadding);
        WRITE_STYLE(IndentSpacing);
        WRITE_STYLE(ScrollbarSize);
        WRITE_STYLE(GrabMinSize);
        // Borders
        WRITE_STYLE(WindowBorderSize);
        WRITE_STYLE(ChildBorderSize);
        WRITE_STYLE(PopupBorderSize);
        WRITE_STYLE(FrameBorderSize);
        WRITE_STYLE(TabBorderSize);
        // Rounding
        WRITE_STYLE(WindowRounding);
        WRITE_STYLE(ChildRounding);
        WRITE_STYLE(FrameRounding);
        WRITE_STYLE(PopupRounding);
        WRITE_STYLE(ScrollbarRounding);
        WRITE_STYLE(GrabRounding);
        WRITE_STYLE(LogSliderDeadzone);
        WRITE_STYLE(TabRounding);
        // Aligment
        WRITE_STYLE_OBJ(WindowTitleAlign);
        WRITE_STYLE(WindowMenuButtonPosition);
        WRITE_STYLE(ColorButtonPosition);
        WRITE_STYLE_OBJ(ButtonTextAlign);
        WRITE_STYLE_OBJ(SelectableTextAlign);
        WRITE_STYLE_OBJ(DisplaySafeAreaPadding);
    	
#undef WRITE_STYLE_OBJ
#undef WRITE_STYLE
    }
	
    // Save 'Colors' tab
    {
        const auto& o = ImGui::GetStyle();
        const ImGuiStyle dummy{};
        auto& colorsJson = j["Colors"];
        for (int i = 0; i < ImGuiCol_COUNT; i++)
            to_json(colorsJson[ImGui::GetStyleColorName(i)], o.Colors[i], dummy.Colors[i]);
    }

	// Save 'Rendering' tab
    {
        const auto& o = ImGui::GetStyle();
        const ImGuiStyle dummy{};
        auto& renderingJson = j["Rendering"];
#define WRITE_STYLE(valueName) if (!(o.valueName == dummy.valueName)) renderingJson[#valueName] = o.valueName

        WRITE_STYLE(AntiAliasedLines);
        WRITE_STYLE(AntiAliasedLinesUseTex);
        WRITE_STYLE(AntiAliasedFill);
        WRITE_STYLE(CurveTessellationTol);
        WRITE_STYLE(CircleTessellationMaxError);
        WRITE_STYLE(Alpha);
        
#undef WRITE_STYLE
    }

    return j;
}

void Style::fromJSON(const json& j) noexcept
{
    // Save window size
    read<value_t::object>(j, "Window Size", ImGui::GetStyle().WindowMinSize);
	
    // Load styleColors struct
    if (j.contains("Current Color"))
        if (const auto& val = j["Current Color"]; val.is_string())
        {
            styleConfig.colors = val.get<std::string>();
#ifndef __APPLE__
            colors = std::distance(avaliableColors.begin(), std::ranges::find(avaliableColors, styleConfig.colors));
#else
            colors = std::distance(avaliableColors.begin(), std::find(avaliableColors.begin(), avaliableColors.end(), styleConfig.colors));
#endif
            UpdateColors(colors, styleConfig.applyStyle.at(colors), styleConfig.applyLerp.at(colors));
        }
    // Apply Style
    if (j.contains("Apply Style") && j["Apply Style"].is_object()) {
        const auto& applyStyleJson = j["Apply Style"];

        for (std::size_t i = 0, e = styleConfig.applyStyle.size(); i != e; ++i)
            if (const char* name = avaliableColors.at(i); applyStyleJson.contains(name))
                read<value_t::boolean>(applyStyleJson, name, styleConfig.applyStyle.at(i));
        if (avaliableColorsStyle.at(colors))
            if (styleConfig.applyStyle.at(colors))
                UpdateColors(colors, styleConfig.applyStyle.at(colors), styleConfig.applyLerp.at(colors));
    }
    // Apply Lerp
    if (j.contains("Apply Lerp") && j["Apply Lerp"].is_object()) {
        const auto& applyLerpJson = j["Apply Lerp"];

        for (std::size_t i = 0, e = styleConfig.applyLerp.size(); i != e; ++i)
            if (const char* name = avaliableColors.at(i); applyLerpJson.contains(name))
                read<value_t::boolean>(applyLerpJson, name, styleConfig.applyLerp.at(i));
        if (avaliableColorsLerp.at(colors))
            if (styleConfig.applyLerp.at(colors))
                UpdateColors(colors, styleConfig.applyStyle.at(colors), styleConfig.applyLerp.at(colors));
    }
    if (j.contains("Current Font"))
        if (const auto& val = j["Current Font"]; val.is_string())
        {
            styleConfig.fonts = val.get<std::string>();
            for (auto& io = ImGui::GetIO(); auto font : io.Fonts->Fonts)
                if (styleConfig.fonts == font->GetDebugName())
                    io.FontDefault = font;
        }
	
    read_number(j, "Alpha Flags", styleConfig.alpha_flags);

    read<value_t::object>(j, "Background Color", styleConfig.backgroundColor);
    read<value_t::object>(j, "Text Color", styleConfig.textColor);
    read<value_t::object>(j, "Main Color", styleConfig.mainColor);
    read<value_t::object>(j, "Main Accent Color", styleConfig.mainAccentColor);
    read<value_t::object>(j, "Highlight Color", styleConfig.highlightColor);
	
    auto& style = ImGui::GetStyle();
    // Load 'Sizes' tab
    if (j.contains("Sizes"))
        if (const auto& val = j["Sizes"]; val.is_object()) {

            // Main
            read<value_t::object>(val, "WindowPadding", style.WindowPadding);
            read<value_t::object>(val, "FramePadding", style.FramePadding);
            read<value_t::object>(val, "CellPadding", style.CellPadding);
            read<value_t::object>(val, "ItemSpacing", style.ItemSpacing);
            read<value_t::object>(val, "ItemInnerSpacing", style.ItemInnerSpacing);
            read<value_t::object>(val, "TouchExtraPadding", style.TouchExtraPadding);
            read_number(val, "IndentSpacing", style.IndentSpacing);
            read_number(val, "ScrollbarSize", style.ScrollbarSize);
            read_number(val, "GrabMinSize", style.GrabMinSize);
            // Borders
            read_number(val, "WindowBorderSize", style.WindowBorderSize);
            read_number(val, "ChildBorderSize", style.ChildBorderSize);
            read_number(val, "PopupBorderSize", style.PopupBorderSize);
            read_number(val, "FrameBorderSize", style.FrameBorderSize);
            read_number(val, "TabBorderSize", style.TabBorderSize);
            // Rounding
            read_number(val, "WindowRounding", style.WindowRounding);
            read_number(val, "ChildRounding", style.ChildRounding);
            read_number(val, "FrameRounding", style.FrameRounding);
            read_number(val, "PopupRounding", style.PopupRounding);
            read_number(val, "ScrollbarRounding", style.ScrollbarRounding);
            read_number(val, "GrabRounding", style.GrabRounding);
            read_number(val, "LogSliderDeadzone", style.LogSliderDeadzone);
            read_number(val, "TabRounding", style.TabRounding);
            // Aligment
            read<value_t::object>(val, "WindowTitleAlign", style.WindowTitleAlign);
            read_number(val, "WindowMenuButtonPosition", style.WindowMenuButtonPosition);
            read_number(val, "ColorButtonPosition", style.ColorButtonPosition);
            read<value_t::object>(val, "SelectableTextAlign", style.SelectableTextAlign);
            read<value_t::object>(val, "DisplaySafeAreaPadding", style.DisplaySafeAreaPadding);
        }
    
    // Load 'Colors' tab
    if (j.contains("Colors"))
        if (const auto& val = j["Colors"]; val.is_object())
            for (int i = 0; i < ImGuiCol_COUNT; i++)
                if (const char* name = ImGui::GetStyleColorName(i); val.contains(name))
                {
                    //read<value_t::object>(val, ImGui::GetStyleColorName(i), style.Colors[i]);
                    auto& color = val[name];
                    const ImGuiStyle dummy{};
                	
                    if (color.contains("X")) read_number(color["X"], "X", style.Colors[i].x);
                    else style.Colors[i].x = dummy.Colors[i].x;
                	
                    if (color.contains("Y")) read_number(color["Y"], "Y", style.Colors[i].y);
                    else style.Colors[i].y = dummy.Colors[i].y;
                	
                    if (color.contains("Z")) read_number(color["Z"], "Z", style.Colors[i].z);
                    else style.Colors[i].z = dummy.Colors[i].z;
                	
                    if (color.contains("W")) read_number(color["W"], "W", style.Colors[i].w);
                    else style.Colors[i].w = dummy.Colors[i].w;
                }

    // Load 'Rendering' tab
    if (j.contains("Rendering"))
        if (const auto& val = j["Rendering"]; val.is_object()) {

            read<value_t::boolean>(val, "AntiAliasedLines", style.AntiAliasedLines);
            read<value_t::boolean>(val, "AntiAliasedLinesUseTex", style.AntiAliasedLinesUseTex);
            read<value_t::boolean>(val, "AntiAliasedFill", style.AntiAliasedFill);
            read_number(val, "CurveTessellationTol", style.CurveTessellationTol);
            read_number(val, "CircleTessellationMaxError", style.CircleTessellationMaxError);
            read_number(val, "Alpha", style.Alpha);
        }
}

void Style::drawGUI()
{
    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);

    if (ImGui::BeginCombo("Colors##Selector", styleConfig.colors.c_str()))
    {
        for (auto color : avaliableColors)
        {
            ImGui::PushID(color);
            if (ImGui::Selectable(color, color == styleConfig.colors.c_str()))
            {
                styleConfig.colors = color;
#ifndef __APPLE__
                const auto it = std::ranges::find(avaliableColors, color);
#else
                const auto it = std::find(avaliableColors.begin(), avaliableColors.end(), color);
#endif
                colors = std::distance(avaliableColors.begin(), it);
                UpdateColors(colors, styleConfig.applyStyle.at(colors), styleConfig.applyLerp.at(colors));
            }
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    
    if (avaliableColorsStyle.at(colors))
    {
        ImGui::SameLine();
        if (ImGui::Checkbox("Style", &styleConfig.applyStyle.at(colors)))
            UpdateColors(colors, styleConfig.applyStyle.at(colors), styleConfig.applyLerp.at(colors));
        ImGui::SameLine();
        Helpers::HelpMarker("Apply additional style settings");
    }

    if (avaliableColorsLerp.at(colors))
    {
        ImGui::SameLine();
        if (ImGui::Checkbox("Lerp", &styleConfig.applyLerp.at(colors)))
            UpdateColors(colors, styleConfig.applyStyle.at(colors), styleConfig.applyLerp.at(colors));
        ImGui::SameLine();
        Helpers::HelpMarker("Apply automatically calculated colors for:\nTab\nTabHovered\nTabActive\nTabUnfocused\nTabUnfocusedActive");
    }

	
    if (ImGui::BeginCombo("Fonts##Selector", styleConfig.fonts.c_str()))
    {
        for (ImGuiIO& io = ImGui::GetIO();
            auto font : io.Fonts->Fonts)
        {
            ImGui::PushID(font);
            if (ImGui::Selectable(font->GetDebugName(), font->GetDebugName() == styleConfig.fonts))
            {
                io.FontDefault = font;
                styleConfig.fonts = font->GetDebugName();
            }
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }

    ImGui::Separator();

    if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoTooltip))
    {
        ImGuiStyle& style = ImGui::GetStyle();
          
        if (ImGui::BeginTabItem("Sizes"))
        {
            ImGui::Text("Main");
            ImGui::SliderFloat2("WindowPadding", reinterpret_cast<float*>(&style.WindowPadding), 0.f, 20.f, "%.f");
            ImGui::SliderFloat2("FramePadding", reinterpret_cast<float*>(&style.FramePadding), 0.f, 20.f, "%.f");
            ImGui::SliderFloat2("CellPadding", reinterpret_cast<float*>(&style.CellPadding), 0.f, 20.f, "%.f");
            ImGui::SliderFloat2("ItemSpacing", reinterpret_cast<float*>(&style.ItemSpacing), 0.f, 20.f, "%.f");
            ImGui::SliderFloat2("ItemInnerSpacing", reinterpret_cast<float*>(&style.ItemInnerSpacing), 0.f, 20.f, "%.f");
            ImGui::SliderFloat2("TouchExtraPadding", reinterpret_cast<float*>(&style.TouchExtraPadding), 0.f, 10.f, "%.f");
            ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.f, 30.f, "%.f");
            ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.f, 20.f, "%.f");
            ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.f, 20.f, "%.f");
            ImGui::Text("Borders");
            ImGui::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.f, 1.f, "%.f");
            ImGui::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.f, 1.f, "%.f");
            ImGui::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.f, 1.f, "%.f");
            ImGui::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.f, 1.f, "%.f");
            ImGui::SliderFloat("TabBorderSize", &style.TabBorderSize, 0.f, 1.f, "%.f");
            ImGui::Text("Rounding");
            ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.f, 12.f, "%.f");
            ImGui::SliderFloat("ChildRounding", &style.ChildRounding, 0.f, 12.f, "%.f");
            ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.f, 12.f, "%.f");
            ImGui::SliderFloat("PopupRounding", &style.PopupRounding, 0.f, 12.f, "%.f");
            ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.f, 12.f, "%.f");
            ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.f, 12.f, "%.f");
            ImGui::SliderFloat("LogSliderDeadzone", &style.LogSliderDeadzone, 0.f, 12.f, "%.f");
            ImGui::SliderFloat("TabRounding", &style.TabRounding, 0.f, 12.f, "%.f");
            ImGui::Text("Alignment");
            ImGui::SliderFloat2("WindowTitleAlign", reinterpret_cast<float*>(&style.WindowTitleAlign), 0.f, 1.f, "%.2f");
            int window_menu_button_position = style.WindowMenuButtonPosition + 1;
            if (ImGui::Combo("WindowMenuButtonPosition", &window_menu_button_position, "None\0Left\0Right\0"))
                style.WindowMenuButtonPosition = window_menu_button_position - 1;
            ImGui::Combo("ColorButtonPosition", &style.ColorButtonPosition, "Left\0Right\0");
            ImGui::SliderFloat2("ButtonTextAlign", reinterpret_cast<float*>(&style.ButtonTextAlign), 0.f, 1.f, "%.2f");
            ImGui::SameLine();
            Helpers::HelpMarker("Alignment applies when a button is larger than its text content.");
            ImGui::SliderFloat2("SelectableTextAlign", reinterpret_cast<float*>(&style.SelectableTextAlign), 0.f, 1.f, "%.2f");
            ImGui::SameLine();
            Helpers::HelpMarker("Alignment applies when a selectable is larger than its text content.");
            ImGui::Text("Safe Area Padding");
            ImGui::SameLine();
            Helpers::HelpMarker("Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured).");
            ImGui::SliderFloat2("DisplaySafeAreaPadding", reinterpret_cast<float*>(&style.DisplaySafeAreaPadding), 0.f, 30.f, "%.f");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Colors"))
        {
            static ImGuiTextFilter filter;
            filter.Draw("Filter colors", ImGui::GetFontSize() * 16);
            
            if (ImGui::RadioButton("Opaque", styleConfig.alpha_flags == ImGuiColorEditFlags_None)) { styleConfig.alpha_flags = ImGuiColorEditFlags_None; } ImGui::SameLine();
            if (ImGui::RadioButton("Alpha", styleConfig.alpha_flags == ImGuiColorEditFlags_AlphaPreview)) { styleConfig.alpha_flags = ImGuiColorEditFlags_AlphaPreview; } ImGui::SameLine();
            if (ImGui::RadioButton("Both", styleConfig.alpha_flags == ImGuiColorEditFlags_AlphaPreviewHalf)) { styleConfig.alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf; } ImGui::SameLine();
            Helpers::HelpMarker(
                "In the color list:\n"
                "Left-click on color square to open color picker,\n"
                "Right-click to open edit options menu.");

            if (styleConfig.colors == "Custom (Easy)")
                ImGui::Text("Go to \"coolors.co\" to generate beautiful palettes");

            ImGui::BeginChild("##colors", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
            ImGui::PushItemWidth(-160);
            if (styleConfig.colors == "Custom (Easy)")
            {
                ImGui::PushID(&styleConfig.backgroundColor);
                if (ImGui::ColorEdit4("Background Color", reinterpret_cast<float*>(&styleConfig.backgroundColor), ImGuiColorEditFlags_AlphaBar | styleConfig.alpha_flags)) UpdateColors(colors);
                ImGui::PopID();
                ImGui::PushID(&styleConfig.textColor);
                if (ImGui::ColorEdit4("Text Color", reinterpret_cast<float*>(&styleConfig.textColor), ImGuiColorEditFlags_AlphaBar | styleConfig.alpha_flags)) UpdateColors(colors);
                ImGui::PopID();
                ImGui::PushID(&styleConfig.mainColor);
                if (ImGui::ColorEdit4("Main Color", reinterpret_cast<float*>(&styleConfig.mainColor), ImGuiColorEditFlags_AlphaBar | styleConfig.alpha_flags)) UpdateColors(colors);
                ImGui::PopID();
                ImGui::PushID(&styleConfig.mainAccentColor);
                if (ImGui::ColorEdit4("Main Accent Color", reinterpret_cast<float*>(&styleConfig.mainAccentColor), ImGuiColorEditFlags_AlphaBar | styleConfig.alpha_flags)) UpdateColors(colors);
                ImGui::PopID();
                ImGui::PushID(&styleConfig.highlightColor);
                if (ImGui::ColorEdit4("Highlight Color", reinterpret_cast<float*>(&styleConfig.highlightColor), ImGuiColorEditFlags_AlphaBar | styleConfig.alpha_flags)) UpdateColors(colors);
                ImGui::PopID();
            }
            else
                for (int i = 0; i < ImGuiCol_COUNT; i++)
                {
                    const char* name = ImGui::GetStyleColorName(i);
                    if (!filter.PassFilter(name))
                        continue;
                    ImGui::PushID(i);
                    ImGui::ColorEdit4("##color", reinterpret_cast<float*>(&style.Colors[i]), ImGuiColorEditFlags_AlphaBar | styleConfig.alpha_flags);
                    ImGui::SameLine(0.f, style.ItemInnerSpacing.x);
                    ImGui::TextUnformatted(name);
                    ImGui::PopID();
                }
            ImGui::PopItemWidth();
            ImGui::EndChild();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Fonts"))
        {
            ImGuiIO& io = ImGui::GetIO();
            ImFontAtlas* atlas = io.Fonts;
            Helpers::HelpMarker("Read FAQ and docs/FONTS.md for details on font loading.");
            ImGui::PushItemWidth(120);
            for (auto fontIterator : atlas->Fonts)
            {
                ImGui::PushID(fontIterator);
                NodeFont(fontIterator);
                ImGui::PopID();
            }
            if (ImGui::TreeNode("Atlas texture", "Atlas texture (%dx%d pixels)", atlas->TexWidth, atlas->TexHeight))
            {
                const auto tint_col = ImVec4(1.f, 1.f, 1.f, 1.f);
                const auto border_col = ImVec4(1.f, 1.f, 1.f, 0.5f);
                ImGui::Image(atlas->TexID, ImVec2(static_cast<float>(atlas->TexWidth), static_cast<float>(atlas->TexHeight)), ImVec2(0, 0), ImVec2(1, 1), tint_col, border_col);
                ImGui::TreePop();
            }

            // Post-baking font scaling. Note that this is NOT the nice way of scaling fonts, read below.
            // (we enforce hard clamping manually as by default DragFloat/SliderFloat allows CTRL+Click text to get out of bounds).
            const float MIN_SCALE = 0.3f;
            const float MAX_SCALE = 2.f;
            Helpers::HelpMarker(
                "Those are old settings provided for convenience.\n"
                "However, the _correct_ way of scaling your UI is currently to reload your font at the designed size, "
                "rebuild the font atlas, and call style.ScaleAllSizes() on a reference ImGuiStyle structure.\n"
                "Using those settings here will give you poor quality results.");
            static float window_scale = 1.f;
            if (ImGui::DragFloat("window scale", &window_scale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp)) // Scale only this window
                ImGui::SetWindowFontScale(window_scale);
            ImGui::DragFloat("global scale", &io.FontGlobalScale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp); // Scale everything
            ImGui::PopItemWidth();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Rendering"))
        {
            ImGui::Checkbox("Anti-aliased lines", &style.AntiAliasedLines);
            ImGui::SameLine();
            Helpers::HelpMarker("When disabling anti-aliasing lines, you'll probably want to disable borders in your style as well.");

            ImGui::Checkbox("Anti-aliased lines use texture", &style.AntiAliasedLinesUseTex);
            ImGui::SameLine();
            Helpers::HelpMarker("Faster lines using texture data. Require backend to render with bilinear filtering (not point/nearest filtering).");

            ImGui::Checkbox("Anti-aliased fill", &style.AntiAliasedFill);
            ImGui::PushItemWidth(100);
            ImGui::DragFloat("Curve Tessellation Tolerance", &style.CurveTessellationTol, 0.02f, 0.1f, 10.f, "%.2f");
            if (style.CurveTessellationTol < 0.1f) style.CurveTessellationTol = 0.1f;

            // When editing the "Circle Segment Max Error" value, draw a preview of its effect on auto-tessellated circles.
            ImGui::DragFloat("Circle Tessellation Max Error", &style.CircleTessellationMaxError, 0.005f, 0.1f, 5.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            if (ImGui::IsItemActive())
            {
                ImGui::SetNextWindowPos(ImGui::GetCursorScreenPos());
                ImGui::BeginTooltip();
                ImGui::TextUnformatted("(R = radius, N = number of segments)");
                ImGui::Spacing();
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                const float min_widget_width = ImGui::CalcTextSize("N: MMM\nR: MMM").x;
                for (int n = 0; n < 8; n++)
                {
                    const float RAD_MIN = 5.f;
                    const float RAD_MAX = 70.f;
                    const float rad = RAD_MIN + (RAD_MAX - RAD_MIN) * static_cast<float>(n) / (8.f - 1.f);

                    ImGui::BeginGroup();

                    ImGui::Text("R: %.f\nN: %d", rad, draw_list->_CalcCircleAutoSegmentCount(rad));

                    const float canvas_width = ImMax(min_widget_width, rad * 2.f);
                    const float offset_x = floorf(canvas_width * 0.5f);
                    const float offset_y = floorf(RAD_MAX);

                    const ImVec2 pos = ImGui::GetCursorScreenPos();
                    draw_list->AddCircle/*Filled*/(ImVec2(pos.x + offset_x, pos.y + offset_y), rad, ImGui::GetColorU32(ImGuiCol_Text));
                    ImGui::Dummy(ImVec2(canvas_width, RAD_MAX * 2));

                    ImGui::EndGroup();
                    ImGui::SameLine();
                }
                ImGui::EndTooltip();
            }
            ImGui::SameLine();
            Helpers::HelpMarker("When drawing circle primitives with \"num_segments == 0\" tesselation will be calculated automatically.");

            //ImGui::DragFloat("Global Alpha", &style.Alpha, 0.005f, 0.2f, 1.f, "%.2f"); // Not exposing zero here so user doesn't "lose" the UI (zero alpha clips all widgets). But application code could have a toggle to switch between zero and non-zero.
            ImGui::PopItemWidth();

            ImGui::EndTabItem();
        }

        if (ImGui::TabItemButton("Reset All"))
        {
            const decltype(styleConfig) dummy;
            styleConfig.colors = dummy.colors;
            styleConfig.fonts = dummy.fonts;
            styleConfig.applyStyle = dummy.applyStyle;
            styleConfig.applyLerp = dummy.applyLerp;
            styleConfig.alpha_flags = dummy.alpha_flags;
        	
            styleConfig.backgroundColor = dummy.backgroundColor;
            styleConfig.textColor = dummy.textColor;
            styleConfig.mainColor = dummy.mainColor;
            styleConfig.mainAccentColor = dummy.mainAccentColor;
            styleConfig.highlightColor = dummy.highlightColor;

            ImGui::GetStyle() = ImGuiStyle();
            ImGui::StyleColorsClassic();
            style.ScrollbarSize = 13.0f;
            style.WindowTitleAlign = { 0.5f, 0.5f };
            style.Colors[ImGuiCol_WindowBg].w = 0.8f;
            style.WindowMinSize = { 600.f, 350.f };
            for (auto& io = ImGui::GetIO(); auto font : io.Fonts->Fonts)
                if (styleConfig.fonts == font->GetDebugName())
                    io.FontDefault = font;
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted("Will reset all settnigs on 'Style' tab");
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }

        ImGui::EndTabBar();
    }

    ImGui::PopItemWidth();
}