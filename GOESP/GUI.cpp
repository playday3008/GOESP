#include "GUI.h"

#include "imgui/imgui.h"

#include "Config.h"
#include "Hooks.h"
#include "ImGuiCustom.h"

#include <array>
#include <vector>

GUI::GUI() noexcept
{
    ImGui::StyleColorsClassic();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScrollbarSize = 13.0f;
    style.WindowTitleAlign = { 0.5f, 0.5f };
    style.Colors[ImGuiCol_WindowBg].w = 0.8f;

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.Fonts->Flags |= ImFontAtlasFlags_NoPowerOfTwoHeight;
    io.Fonts->AddFontDefault();
}

static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void GUI::updateColors() const noexcept
{
    ImGuiStyle& style = ImGui::GetStyle();
    switch (config->menuColors) {
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
            style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.260f, 0.590f, 0.980f, 1.f);
    break;
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




            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.09f, 0.27f, 0.27f, 0.67f);
    break;
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




            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
    break;
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




            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
    break;
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




            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
    break;
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




            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(57.f / 255.f, 79.f / 255.f, 105.f / 255.f, 0.73f);
    break;
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

            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    break;
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

            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    break;
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




            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.200f, 0.220f, 0.270f, 0.73f);
    break;
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

            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
    break;
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
            style.Colors[ImGuiCol_ModalWindowDimBg] = style.Colors[ImGuiCol_NavWindowingDimBg];
    break;
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
            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    break;
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

            style.WindowMenuButtonPosition = ImGuiDir_Right;
    break;
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
            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    break;
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
            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    break;
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
            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    break;
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
            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    break;
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


            style.Colors[ImGuiCol_ModalWindowDimBg] = style.Colors[ImGuiCol_WindowBg];
    break;
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
            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    break;
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




            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
    break;
#pragma endregion
    }
}

void GUI::render() noexcept
{
    if (!open)
        return;

    ImGui::Begin(
        "GOESP "
        "BETA "
        "for "
#ifdef _WIN32
        "Windows"
#elif __linux__
        "Linux"
#elif __APPLE__
        "macOS"
#else
    #error("Unsupported platform!")
#endif
        " by PlayDay"
        , nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    if (!ImGui::BeginTabBar("##tabbar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoTooltip)) {
        ImGui::End();
        return;
    }

    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 350.0f);

    ImGui::TextUnformatted("Build date: " __DATE__ " " __TIME__);
    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 55.0f);

    if (ImGui::Button("Unload"))
        hooks->uninstall();

    if (ImGui::BeginTabItem("ESP")) {
        drawESPTab();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Misc")) {
        ImGuiCustom::colorPicker("Reload Progress", config->reloadProgress);
        ImGuiCustom::colorPicker("Recoil Crosshair", config->recoilCrosshair);
        ImGuiCustom::colorPicker("Noscope Crosshair", config->noscopeCrosshair);
        ImGui::Checkbox("Purchase List", &config->purchaseList.enabled);
        ImGui::SameLine();

        ImGui::PushID("Purchase List");
        if (ImGui::Button("..."))
            ImGui::OpenPopup("");

        if (ImGui::BeginPopup("")) {
            ImGui::SetNextItemWidth(75.0f);
            ImGui::Combo("Mode", &config->purchaseList.mode, "Details\0Summary\0");
            ImGui::Checkbox("Only During Freeze Time", &config->purchaseList.onlyDuringFreezeTime);
            ImGui::Checkbox("Show Prices", &config->purchaseList.showPrices);
            ImGui::Checkbox("No Title Bar", &config->purchaseList.noTitleBar);
            ImGui::EndPopup();
        }
        ImGui::PopID();

        ImGui::PushID("Observer List");
        ImGui::Checkbox("Observer List", &config->observerList.enabled);
        ImGui::SameLine();

        if (ImGui::Button("..."))
            ImGui::OpenPopup("");

        if (ImGui::BeginPopup("")) {
            ImGui::Checkbox("No Title Bar", &config->observerList.noTitleBar);
            ImGui::EndPopup();
        }
        ImGui::PopID();

        ImGui::Checkbox("Ignore Flashbang", &config->ignoreFlashbang);
        ImGui::Checkbox("FPS Counter", &config->fpsCounter.enabled);
        ImGuiCustom::colorPicker("Offscreen Enemies", config->offscreenEnemies.color, &config->offscreenEnemies.enabled);

        ImGuiCustom::colorPicker("Rainbow Bar", config->rainbowBar);
        if (config->rainbowBar.enabled) {
            ImGui::SameLine();
            ImGui::PushID("Rainbow Bar");
            if (ImGui::Button("..."))
                ImGui::OpenPopup("RB");

            if (ImGui::BeginPopup("RB")) {
                ImGui::Text("Position:");
                ImGui::Checkbox("Upper", &config->rainbowUp);
                ImGui::Checkbox("Bottom", &config->rainbowBottom);
                ImGui::Checkbox("Left", &config->rainbowLeft);
                ImGui::Checkbox("Right", &config->rainbowRight);
                ImGui::Text("Scale:");
                ImGui::SliderFloat("Scale", &config->rainbowScale, 0.03125f, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic);
                ImGui::Text("Scale presets:");
                if (ImGui::Button("0.25x"))
                    config->rainbowScale = 0.03125f;
                ImGui::SameLine();
                if (ImGui::Button("0.5x"))
                    config->rainbowScale = 0.0625f;
                ImGui::SameLine();
                if (ImGui::Button("1x"))
                    config->rainbowScale = 0.125f;
                ImGui::SameLine();
                if (ImGui::Button("2x"))
                    config->rainbowScale = 0.25f;
                ImGui::SameLine();
                if (ImGui::Button("4x"))
                    config->rainbowScale = 0.5f;
                ImGui::SameLine();
                if (ImGui::Button("8x"))
                    config->rainbowScale = 1.0f;
                ImGui::Text("Pulse:");
                ImGui::Checkbox("Enable", &config->rainbowPulse);
                ImGui::SliderFloat("Speed", &config->rainbowPulseSpeed, 0.1f, 25.0f, "%.1f", ImGuiSliderFlags_Logarithmic);
                ImGui::EndPopup();
            }
            ImGui::PopID();
        }
        ImGuiCustom::colorPicker("Watermark", config->watermark);
        if (config->watermark.enabled) {
            ImGui::SameLine();
            ImGui::PushID("Watermark");
            if (ImGui::Button("..."))
                ImGui::OpenPopup("WM");

            if (ImGui::BeginPopup("WM")) {
                ImGui::Checkbox("Nickname (Only in game)", &config->watermarkNickname);
                ImGui::Checkbox("Username", &config->watermarkUsername);
                ImGui::Checkbox("FPS", &config->watermarkFPS);
                ImGui::Checkbox("Ping", &config->watermarkPing);
                ImGui::Checkbox("Tickrate", &config->watermarkTickrate);
                ImGui::Checkbox("Time", &config->watermarkTime);
                ImGui::DragFloat("Scale", &config->watermarkScale, 0.005f, 0.3f, 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                ImGui::EndPopup();
            }
            ImGui::PopID();
        }

        ImGuiCustom::colorPicker("Hit marker", config->hitMarker);
        config->hitMarker.thickness = std::clamp<float>(config->hitMarker.thickness, 0.f, 10.f);
        if (config->hitMarker.enabled) {
            ImGui::SameLine();
            ImGui::PushID("Hit marker");
            if (ImGui::Button("..."))
                ImGui::OpenPopup("HM");

            if (ImGui::BeginPopup("HM")) {
                float hitMarkerLength = config->hitMarkerLength + 4.f;
                if (ImGui::SliderFloat("Hit Marker Length", &hitMarkerLength, 1.f, 16.f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
                    config->hitMarkerLength = hitMarkerLength - 4.f;
                ImGui::SliderFloat("Hit marker time", &config->hitMarkerTime, 0.1f, 1.5f, "%.2fs");
                ImGui::EndPopup();
            }
            ImGui::PopID();
        }
        ImGuiCustom::colorPicker("Hit marker damage indicator", config->hitMarkerDamageIndicator);
        if (config->hitMarkerDamageIndicator.enabled) {
            ImGui::SameLine();
            ImGui::PushID("Hit marker damage indicator");
            if (ImGui::Button("..."))
                ImGui::OpenPopup("HMDI");

            if (ImGui::BeginPopup("HMDI")) {
                ImGui::Checkbox("Customize Hitmarker", &config->hitMarkerDamageIndicatorCustomize);
                if (config->hitMarkerDamageIndicatorCustomize) {
                    ImGui::SliderInt(" ", &config->hitMarkerDamageIndicatorDist, 1, 100, "Dist: %d");
                    ImGui::PushID(1);
                    ImGui::SliderFloat(" ", &config->hitMarkerDamageIndicatorRatio, 0.1f, 1.0f, "Ratio: %.2f");
                    ImGui::PopID();
                };
                ImGui::EndPopup();
            }
            ImGui::PopID();
        };

        if (ImGui::CollapsingHeader("Style Configuration")) {
            if (ImGui::Combo("Menu colors", &config->menuColors, 
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

            if (config->menuColors == 3) {
                ImGuiStyle& style = ImGui::GetStyle();
                for (int i = 0; i < ImGuiCol_COUNT; i++) {
                    if (i && i & 3) ImGui::SameLine(220.0f * (i & 3));

                    ImGuiCustom::colorPopup(ImGui::GetStyleColorName(i), (std::array<float, 4>&)style.Colors[i]);
                }
            }
            // Custom (Easy)
            else if (config->menuColors == 4) {

                ImGuiStyle& style = ImGui::GetStyle();

                config->customEasy.BackGroundColor.color[0] = style.Colors[ImGuiCol_WindowBg].x;
                config->customEasy.BackGroundColor.color[1] = style.Colors[ImGuiCol_WindowBg].y;
                config->customEasy.BackGroundColor.color[2] = style.Colors[ImGuiCol_WindowBg].z;
                config->customEasy.TextColor.color[0] = style.Colors[ImGuiCol_Text].x;
                config->customEasy.TextColor.color[1] = style.Colors[ImGuiCol_Text].y;
                config->customEasy.TextColor.color[2] = style.Colors[ImGuiCol_Text].z;
                config->customEasy.MainColor.color[0] = style.Colors[ImGuiCol_Button].x;
                config->customEasy.MainColor.color[1] = style.Colors[ImGuiCol_Button].y;
                config->customEasy.MainColor.color[2] = style.Colors[ImGuiCol_Button].z;
                config->customEasy.MainAccentColor.color[0] = style.Colors[ImGuiCol_Header].x;
                config->customEasy.MainAccentColor.color[1] = style.Colors[ImGuiCol_Header].y;
                config->customEasy.MainAccentColor.color[2] = style.Colors[ImGuiCol_Header].z;
                config->customEasy.HighlightColor.color[0] = style.Colors[ImGuiCol_CheckMark].x;
                config->customEasy.HighlightColor.color[1] = style.Colors[ImGuiCol_CheckMark].y;
                config->customEasy.HighlightColor.color[2] = style.Colors[ImGuiCol_CheckMark].z;

                ImGui::Text("Go to \"coolors.co\" to generate beautiful palettes");
                ImGuiCustom::colorPopup("Background Color", config->customEasy.BackGroundColor.color, 0, 0, 0);
                ImGuiCustom::colorPopup("Text Color", config->customEasy.TextColor.color, 0, 0, 0);
                ImGuiCustom::colorPopup("Main Color", config->customEasy.MainColor.color, 0, 0, 0);
                ImGuiCustom::colorPopup("Main Accent Color", config->customEasy.MainAccentColor.color, 0, 0, 0);
                ImGuiCustom::colorPopup("Highlight Color", config->customEasy.HighlightColor.color, 0, 0, 0);

                auto BackGroundColor = config->customEasy.BackGroundColor.color;
                auto TextColor = config->customEasy.TextColor.color;
                auto MainColor = config->customEasy.MainColor.color;
                auto MainAccentColor = config->customEasy.MainAccentColor.color;
                auto HighlightColor = config->customEasy.HighlightColor.color;

                style.WindowMenuButtonPosition = ImGuiDir_Right;

#define GET_COLOR(c, a) {c[0], c[1], c[2], (a * config->customEasy.c.color[3])};
#define DARKEN(c ,p) ImVec4(std::max(0.f, c.x - 1.0f * p), std::max(0.f, c.y - 1.0f * p), std::max(0.f, c.z - 1.0f *p), c.w);
#define LIGHTEN(x, y, z, w, p) ImVec4(std::max(0.f, x + 1.0f * p), std::max(0.f, y + 1.0f * p), std::max(0.f, z + 1.0f *p), w);
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
                style.Colors[ImGuiCol_FrameBgActive] = ACTIVE(style.Colors[ImGuiCol_FrameBg], (1.f * config->customEasy.MainAccentColor.color[3]));
                style.Colors[ImGuiCol_TitleBg] = style.Colors[ImGuiCol_WindowBg];
                style.Colors[ImGuiCol_TitleBgActive] = ACTIVE(style.Colors[ImGuiCol_TitleBg], (1.f * config->customEasy.BackGroundColor.color[3]));
                style.Colors[ImGuiCol_TitleBgCollapsed] = COLLAPSED(style.Colors[ImGuiCol_TitleBg]);
                style.Colors[ImGuiCol_MenuBarBg] = DARKEN(style.Colors[ImGuiCol_WindowBg], 0.2f);
                style.Colors[ImGuiCol_ScrollbarBg] = LIGHTEN(BackGroundColor[0], BackGroundColor[1], BackGroundColor[2], (0.5f * config->customEasy.BackGroundColor.color[3]), 0.4f);
                style.Colors[ImGuiCol_ScrollbarGrab] = LIGHTEN(style.Colors[ImGuiCol_WindowBg].x, style.Colors[ImGuiCol_WindowBg].y, style.Colors[ImGuiCol_WindowBg].z, style.Colors[ImGuiCol_WindowBg].w, 0.3f);
                style.Colors[ImGuiCol_ScrollbarGrabHovered] = HOVERED(style.Colors[ImGuiCol_ScrollbarGrab]);
                style.Colors[ImGuiCol_ScrollbarGrabActive] = ACTIVE(style.Colors[ImGuiCol_ScrollbarGrab], (1.f * config->customEasy.BackGroundColor.color[3]));
                style.Colors[ImGuiCol_CheckMark] = GET_COLOR(HighlightColor, 0.8f);
                style.Colors[ImGuiCol_SliderGrab] = style.Colors[ImGuiCol_CheckMark];
                style.Colors[ImGuiCol_SliderGrabActive] = ACTIVE(style.Colors[ImGuiCol_SliderGrab], (1.f * config->customEasy.HighlightColor.color[3]));
                style.Colors[ImGuiCol_Button] = GET_COLOR(MainColor, 0.8f);
                style.Colors[ImGuiCol_ButtonHovered] = HOVERED(style.Colors[ImGuiCol_Button]);
                style.Colors[ImGuiCol_ButtonActive] = ACTIVE(style.Colors[ImGuiCol_Button], (1.f * config->customEasy.MainColor.color[3]));
                style.Colors[ImGuiCol_Header] = GET_COLOR(MainAccentColor, 0.8f);
                style.Colors[ImGuiCol_HeaderHovered] = HOVERED(style.Colors[ImGuiCol_Header]);
                style.Colors[ImGuiCol_HeaderActive] = ACTIVE(style.Colors[ImGuiCol_Header], (1.f * config->customEasy.MainAccentColor.color[3]));
                style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
                style.Colors[ImGuiCol_SeparatorHovered] = HOVERED(style.Colors[ImGuiCol_Separator]);
                style.Colors[ImGuiCol_SeparatorActive] = ACTIVE(style.Colors[ImGuiCol_Separator], (1.f * config->customEasy.BackGroundColor.color[3]));
                style.Colors[ImGuiCol_ResizeGrip] = GET_COLOR(MainColor, 0.2f);
                style.Colors[ImGuiCol_ResizeGripHovered] = HOVERED(style.Colors[ImGuiCol_ResizeGrip]);
                style.Colors[ImGuiCol_ResizeGripActive] = ACTIVE(style.Colors[ImGuiCol_ResizeGrip], (1.f * config->customEasy.MainColor.color[3]));
                style.Colors[ImGuiCol_Tab] = GET_COLOR(MainColor, 0.6f);
                style.Colors[ImGuiCol_TabHovered] = HOVERED(style.Colors[ImGuiCol_Tab]);
                style.Colors[ImGuiCol_TabActive] = ACTIVE(style.Colors[ImGuiCol_Tab], (1.f * config->customEasy.MainColor.color[3]));
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

        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Configs")) {
#ifdef _WIN32
        ImGui::TextUnformatted("Config is saved as \"config.txt\" inside GOESP directory in Documents");
#elif __linux__
        ImGui::TextUnformatted("Config is saved as \"config.txt\" inside ~/GOESP directory");
#endif
        if (ImGui::Button("Load"))
            ImGui::OpenPopup("Load confirmation");
        if (ImGui::BeginPopup("Load confirmation")) {
            if (ImGui::Selectable("Confirm"))
                config->load();
            if (ImGui::Selectable("Cancel")) {/*nothing to do*/ }
            ImGui::EndPopup();
        }
        if (ImGui::Button("Save"))
            ImGui::OpenPopup("Save confirmation");
        if (ImGui::BeginPopup("Save confirmation")) {
            if (ImGui::Selectable("Confirm"))
                config->save();
            if (ImGui::Selectable("Cancel")) {/*nothing to do*/ }
            ImGui::EndPopup();
        }
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Info")) {
        ImGui::Text("GOESP by danielkrupinski;");
        ImGui::Text("GOESP BETA by PlayDay (playday3008(GitHub)), (PlayDay#4049);");
        ImGui::Text("Discord by w1ldac3 (https://discord.gg/xWEtQAn);");
        ImGui::Text(" ");
        ImGui::Text("Functions by:");
        ImGui::Text("Rainbow bar by: PlayDay");
        ImGui::Text("Crashhandler support by: PlayDay and W4tev3n");
        ImGui::Text("AntiDetection by: 0xE232FE");
        ImGui::Text("Save/Load confirmation by: PlayDay");
#ifdef _WIN32
        ImGui::Text("BSOD button by: PlayDay");
        ImGui::SameLine();
        if (ImGui::Button("BSOD"))
            ImGui::OpenPopup("Do you want to crash your Windows?");
        if (ImGui::BeginPopup("Do you want to crash your Windows?")) {
            if (ImGui::Selectable("Confirm")) {
                HMODULE ntdll = LoadLibraryA("ntdll");
                FARPROC RtlAdjustPrivilege = GetProcAddress(ntdll, "RtlAdjustPrivilege");
                FARPROC NtRaiseHardError = GetProcAddress(ntdll, "NtRaiseHardError");

                if (RtlAdjustPrivilege != nullptr && NtRaiseHardError != nullptr) {
                    BOOLEAN tmp1; DWORD tmp2;
                    ((void(*)(DWORD, DWORD, BOOLEAN, LPBYTE))RtlAdjustPrivilege)(19, 1, 0, &tmp1);
                    ((void(*)(DWORD, DWORD, DWORD, DWORD, DWORD, LPDWORD))NtRaiseHardError)(0xc0000022, 0, 0, 0, 6, &tmp2);
                }
            }
            if (ImGui::Selectable("Cancel")) {/*nothing to do*/ }
            ImGui::EndPopup();
        }
        ImGui::SameLine(); HelpMarker("WARNING: will crash your windows");
#endif
        ImGui::Text("Watermark by: PlayDay");
        ImGui::SameLine(); HelpMarker("Nickname shown only on map");
        ImGui::Text("Menu Colors by: PlayDay");
        ImGui::SameLine(); HelpMarker("BUG: close collapsing header before switch to Misc/Info tab\n"
            "WIP - Work in Process (incompleted themes)");
        ImGui::Text("Hitmarker by: PlayDay");
        ImGui::Text("Hitmarker Damage Indicator by: ZerGo0, improved by RyDeem;");

        ImGui::Text(" ");

        ImGuiStyle& style = ImGui::GetStyle();
        ImGuiIO& io = ImGui::GetIO();

        if (ImGui::CollapsingHeader("GUI Configuration by: PlayDay"))
        {
            const float MIN_SCALE = 0.3f;
            const float MAX_SCALE = 2.0f;
            ImGui::DragFloat("global scale", &io.FontGlobalScale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp); // Scale everything
            if (ImGui::TreeNode("Sizes##2"))
            {
                ImGui::Text("Main");
                ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
                ImGui::SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f");
                ImGui::SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f");
                ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
                ImGui::SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f");
                ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
                ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
                ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");
                ImGui::Text("Borders");
                ImGui::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, 1.0f, "%.0f");
                ImGui::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f, "%.0f");
                ImGui::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f, "%.0f");
                ImGui::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f, "%.0f");
                ImGui::SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, 1.0f, "%.0f");
                ImGui::Text("Rounding");
                ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");
                ImGui::SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f, "%.0f");
                ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f");
                ImGui::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f, "%.0f");
                ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f");
                ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f");
                ImGui::SliderFloat("LogSliderDeadzone", &style.LogSliderDeadzone, 0.0f, 12.0f, "%.0f");
                ImGui::SliderFloat("TabRounding", &style.TabRounding, 0.0f, 12.0f, "%.0f");
                ImGui::Text("Alignment");
                ImGui::SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f");
                int window_menu_button_position = style.WindowMenuButtonPosition + 1;
                if (ImGui::Combo("WindowMenuButtonPosition", (int*)&window_menu_button_position, "None\0Left\0Right\0"))
                    style.WindowMenuButtonPosition = window_menu_button_position - 1;
                ImGui::Combo("ColorButtonPosition", (int*)&style.ColorButtonPosition, "Left\0Right\0");
                ImGui::SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f");
                ImGui::SameLine(); HelpMarker("Alignment applies when a button is larger than its text content.");
                ImGui::SliderFloat2("SelectableTextAlign", (float*)&style.SelectableTextAlign, 0.0f, 1.0f, "%.2f");
                ImGui::SameLine(); HelpMarker("Alignment applies when a selectable is larger than its text content.");
                ImGui::Text("Safe Area Padding");
                ImGui::SameLine(); HelpMarker("Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured).");
                ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style.DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f");
            }
            if (ImGui::TreeNode("Rendering##2"))
            {
                ImGui::Checkbox("Anti-aliased lines", &style.AntiAliasedLines);
                ImGui::SameLine(); HelpMarker("When disabling anti-aliasing lines, you'll probably want to disable borders in your style as well.");
                ImGui::Checkbox("Anti-aliased lines use texture", &style.AntiAliasedLinesUseTex);
                ImGui::SameLine(); HelpMarker("Faster lines using texture data. Require back-end to render with bilinear filtering (not point/nearest filtering).");
                ImGui::Checkbox("Anti-aliased fill", &style.AntiAliasedFill);
                ImGui::PushItemWidth(100);
                ImGui::DragFloat("Curve Tessellation Tolerance", &style.CurveTessellationTol, 0.02f, 0.10f, 10.0f, "%.2f");
                if (style.CurveTessellationTol < 0.10f) style.CurveTessellationTol = 0.10f;
            }
        }
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
    ImGui::End();
}

void GUI::drawESPTab() noexcept
{
    static std::size_t currentCategory;
    static auto currentItem = "All";

    constexpr auto getConfigShared = [](std::size_t category, const char* item) noexcept -> Shared& {
        switch (category) {
        case 0: default: return config->enemies[item];
        case 1: return config->allies[item];
        case 2: return config->weapons[item];
        case 3: return config->projectiles[item];
        case 4: return config->lootCrates[item];
        case 5: return config->otherEntities[item];
        }
    };

    constexpr auto getConfigPlayer = [](std::size_t category, const char* item) noexcept -> Player& {
        switch (category) {
        case 0: default: return config->enemies[item];
        case 1: return config->allies[item];
        }
    };

    if (ImGui::ListBoxHeader("##list", { 170.0f, 300.0f })) {
        constexpr std::array categories{ "Enemies", "Allies", "Weapons", "Projectiles", "Loot Crates", "Other Entities" };

        for (std::size_t i = 0; i < categories.size(); ++i) {
            if (ImGui::Selectable(categories[i], currentCategory == i && std::string_view{ currentItem } == "All")) {
                currentCategory = i;
                currentItem = "All";
            }

            if (ImGui::BeginDragDropSource()) {
                switch (i) {
                case 0: case 1: ImGui::SetDragDropPayload("Player", &getConfigPlayer(i, "All"), sizeof(Player), ImGuiCond_Once); break;
                case 2: ImGui::SetDragDropPayload("Weapon", &config->weapons["All"], sizeof(Weapon), ImGuiCond_Once); break;
                case 3: ImGui::SetDragDropPayload("Projectile", &config->projectiles["All"], sizeof(Projectile), ImGuiCond_Once); break;
                default: ImGui::SetDragDropPayload("Entity", &getConfigShared(i, "All"), sizeof(Shared), ImGuiCond_Once); break;
                }
                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Player")) {
                    const auto& data = *(Player*)payload->Data;

                    switch (i) {
                    case 0: case 1: getConfigPlayer(i, "All") = data; break;
                    case 2: config->weapons["All"] = data; break;
                    case 3: config->projectiles["All"] = data; break;
                    default: getConfigShared(i, "All") = data; break;
                    }
                }

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Weapon")) {
                    const auto& data = *(Weapon*)payload->Data;

                    switch (i) {
                    case 0: case 1: getConfigPlayer(i, "All") = data; break;
                    case 2: config->weapons["All"] = data; break;
                    case 3: config->projectiles["All"] = data; break;
                    default: getConfigShared(i, "All") = data; break;
                    }
                }

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Projectile")) {
                    const auto& data = *(Projectile*)payload->Data;

                    switch (i) {
                    case 0: case 1: getConfigPlayer(i, "All") = data; break;
                    case 2: config->weapons["All"] = data; break;
                    case 3: config->projectiles["All"] = data; break;
                    default: getConfigShared(i, "All") = data; break;
                    }
                }

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity")) {
                    const auto& data = *(Shared*)payload->Data;

                    switch (i) {
                    case 0: case 1: getConfigPlayer(i, "All") = data; break;
                    case 2: config->weapons["All"] = data; break;
                    case 3: config->projectiles["All"] = data; break;
                    default: getConfigShared(i, "All") = data; break;
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::PushID(i);
            ImGui::Indent();

            const auto items = [](std::size_t category) noexcept -> std::vector<const char*> {
                switch (category) {
                case 0:
                case 1: return { "Visible", "Occluded" };
                case 2: return { "Pistols", "SMGs", "Rifles", "Sniper Rifles", "Shotguns", "Machineguns", "Grenades", "Melee", "Other" };
                case 3: return { "Flashbang", "HE Grenade", "Breach Charge", "Bump Mine", "Decoy Grenade", "Molotov", "TA Grenade", "Smoke Grenade", "Snowball" };
                case 4: return { "Pistol Case", "Light Case", "Heavy Case", "Explosive Case", "Tools Case", "Cash Dufflebag" };
                case 5: return { "Defuse Kit", "Chicken", "Planted C4", "Hostage", "Sentry", "Cash", "Ammo Box", "Radar Jammer", "Snowball Pile" };
                default: return { };
                }
            }(i);

            const auto categoryEnabled = getConfigShared(i, "All").enabled;

            for (std::size_t j = 0; j < items.size(); ++j) {
                static bool selectedSubItem;
                if (!categoryEnabled || getConfigShared(i, items[j]).enabled) {
                    if (ImGui::Selectable(items[j], currentCategory == i && !selectedSubItem && std::string_view{ currentItem } == items[j])) {
                        currentCategory = i;
                        currentItem = items[j];
                        selectedSubItem = false;
                    }

                    if (ImGui::BeginDragDropSource()) {
                        switch (i) {
                        case 0: case 1: ImGui::SetDragDropPayload("Player", &getConfigPlayer(i, items[j]), sizeof(Player), ImGuiCond_Once); break;
                        case 2: ImGui::SetDragDropPayload("Weapon", &config->weapons[items[j]], sizeof(Weapon), ImGuiCond_Once); break;
                        case 3: ImGui::SetDragDropPayload("Projectile", &config->projectiles[items[j]], sizeof(Projectile), ImGuiCond_Once); break;
                        default: ImGui::SetDragDropPayload("Entity", &getConfigShared(i, items[j]), sizeof(Shared), ImGuiCond_Once); break;
                        }
                        ImGui::EndDragDropSource();
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Player")) {
                            const auto& data = *(Player*)payload->Data;

                            switch (i) {
                            case 0: case 1: getConfigPlayer(i, items[j]) = data; break;
                            case 2: config->weapons[items[j]] = data; break;
                            case 3: config->projectiles[items[j]] = data; break;
                            default: getConfigShared(i, items[j]) = data; break;
                            }
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Weapon")) {
                            const auto& data = *(Weapon*)payload->Data;

                            switch (i) {
                            case 0: case 1: getConfigPlayer(i, items[j]) = data; break;
                            case 2: config->weapons[items[j]] = data; break;
                            case 3: config->projectiles[items[j]] = data; break;
                            default: getConfigShared(i, items[j]) = data; break;
                            }
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Projectile")) {
                            const auto& data = *(Projectile*)payload->Data;

                            switch (i) {
                            case 0: case 1: getConfigPlayer(i, items[j]) = data; break;
                            case 2: config->weapons[items[j]] = data; break;
                            case 3: config->projectiles[items[j]] = data; break;
                            default: getConfigShared(i, items[j]) = data; break;
                            }
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity")) {
                            const auto& data = *(Shared*)payload->Data;

                            switch (i) {
                            case 0: case 1: getConfigPlayer(i, items[j]) = data; break;
                            case 2: config->weapons[items[j]] = data; break;
                            case 3: config->projectiles[items[j]] = data; break;
                            default: getConfigShared(i, items[j]) = data; break;
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                if (i != 2)
                    continue;

                ImGui::Indent();

                const auto subItems = [](std::size_t item) noexcept -> std::vector<const char*> {
                    switch (item) {
                    case 0: return { "Glock-18", "P2000", "USP-S", "Dual Berettas", "P250", "Tec-9", "Five-SeveN", "CZ75-Auto", "Desert Eagle", "R8 Revolver" };
                    case 1: return { "MAC-10", "MP9", "MP7", "MP5-SD", "UMP-45", "P90", "PP-Bizon" };
                    case 2: return { "Galil AR", "FAMAS", "AK-47", "M4A4", "M4A1-S", "SG 553", "AUG" };
                    case 3: return { "SSG 08", "AWP", "G3SG1", "SCAR-20" };
                    case 4: return { "Nova", "XM1014", "Sawed-Off", "MAG-7" };
                    case 5: return { "M249", "Negev" };
                    case 6: return { "Flashbang", "HE Grenade", "Smoke Grenade", "Molotov", "Decoy Grenade", "Incendiary", "TA Grenade", "Fire Bomb", "Diversion", "Frag Grenade", "Snowball" };
                    case 7: return { "Axe", "Hammer", "Wrench" };
                    case 8: return { "C4", "Healthshot", "Bump Mine", "Zone Repulsor", "Shield" };
                    default: return { };
                    }
                }(j);

                const auto itemEnabled = getConfigShared(i, items[j]).enabled;

                for (const auto subItem : subItems) {
                    auto& subItemConfig = config->weapons[subItem];
                    if ((categoryEnabled || itemEnabled) && !subItemConfig.enabled)
                        continue;

                    if (ImGui::Selectable(subItem, currentCategory == i && selectedSubItem && std::string_view{ currentItem } == subItem)) {
                        currentCategory = i;
                        currentItem = subItem;
                        selectedSubItem = true;
                    }

                    if (ImGui::BeginDragDropSource()) {
                        ImGui::SetDragDropPayload("Weapon", &subItemConfig, sizeof(Weapon), ImGuiCond_Once);
                        ImGui::EndDragDropSource();
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Player")) {
                            const auto& data = *(Player*)payload->Data;
                            subItemConfig = data;
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Weapon")) {
                            const auto& data = *(Weapon*)payload->Data;
                            subItemConfig = data;
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Projectile")) {
                            const auto& data = *(Projectile*)payload->Data;
                            subItemConfig = data;
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity")) {
                            const auto& data = *(Shared*)payload->Data;
                            subItemConfig = data;
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                ImGui::Unindent();
            }
            ImGui::Unindent();
            ImGui::PopID();
        }
        ImGui::ListBoxFooter();
    }

    ImGui::SameLine();

    if (ImGui::BeginChild("##child", { 400.0f, 0.0f })) {
        auto& sharedConfig = getConfigShared(currentCategory, currentItem);

        ImGui::Checkbox("Enabled", &sharedConfig.enabled);
        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 260.0f);
        ImGui::SetNextItemWidth(220.0f);
        if (ImGui::BeginCombo("Font", config->getSystemFonts()[sharedConfig.font.index].c_str())) {
            for (size_t i = 0; i < config->getSystemFonts().size(); i++) {
                bool isSelected = config->getSystemFonts()[i] == sharedConfig.font.name;
                if (ImGui::Selectable(config->getSystemFonts()[i].c_str(), isSelected, 0, { 250.0f, 0.0f })) {
                    sharedConfig.font.index = i;
                    sharedConfig.font.name = config->getSystemFonts()[i];
                    config->scheduleFontLoad(i);
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();

        constexpr auto spacing = 250.0f;
        ImGuiCustom::colorPicker("Snapline", sharedConfig.snapline);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(90.0f);
        ImGui::Combo("##1", &sharedConfig.snapline.type, "Bottom\0Top\0Crosshair\0");
        ImGui::SameLine(spacing);
        ImGuiCustom::colorPicker("Box", sharedConfig.box);
        ImGui::SameLine();

        ImGui::PushID("Box");

        if (ImGui::Button("..."))
            ImGui::OpenPopup("");

        if (ImGui::BeginPopup("")) {
            ImGui::SetNextItemWidth(95.0f);
            ImGui::Combo("Type", &sharedConfig.box.type, "2D\0" "2D corners\0" "3D\0" "3D corners\0");
            ImGui::SetNextItemWidth(275.0f);
            ImGui::SliderFloat3("Scale", sharedConfig.box.scale.data(), 0.0f, 0.50f, "%.2f");
            ImGuiCustom::colorPicker("Fill", sharedConfig.box.fill);
            ImGui::EndPopup();
        }

        ImGui::PopID();

        ImGuiCustom::colorPicker("Name", sharedConfig.name);
        if (currentCategory <= 3)
            ImGui::SameLine(spacing);

        if (currentCategory < 2) {
            auto& playerConfig = getConfigPlayer(currentCategory, currentItem);

            ImGuiCustom::colorPicker("Weapon", playerConfig.weapon);
            ImGuiCustom::colorPicker("Flash Duration", playerConfig.flashDuration);
            ImGui::SameLine(spacing);
            ImGuiCustom::colorPicker("Skeleton", playerConfig.skeleton);
            ImGui::Checkbox("Audible Only", &playerConfig.audibleOnly);
            ImGui::SameLine(spacing);
            ImGui::Checkbox("Spotted Only", &playerConfig.spottedOnly);

            ImGuiCustom::colorPicker("Head Box", playerConfig.headBox);
            ImGui::SameLine();

            ImGui::PushID("Head Box");

            if (ImGui::Button("..."))
                ImGui::OpenPopup("");

            if (ImGui::BeginPopup("")) {
                ImGui::SetNextItemWidth(95.0f);
                ImGui::Combo("Type", &playerConfig.headBox.type, "2D\0" "2D corners\0" "3D\0" "3D corners\0");
                ImGui::SetNextItemWidth(275.0f);
                ImGui::SliderFloat3("Scale", playerConfig.headBox.scale.data(), 0.0f, 0.50f, "%.2f");
                ImGuiCustom::colorPicker("Fill", playerConfig.headBox.fill);
                ImGui::EndPopup();
            }

            ImGui::PopID();

            ImGui::SameLine(spacing);
            ImGui::Checkbox("Health Bar", &playerConfig.healthBar);
        } else if (currentCategory == 2) {
            auto& weaponConfig = config->weapons[currentItem];
            ImGuiCustom::colorPicker("Ammo", weaponConfig.ammo);
        } else if (currentCategory == 3) {
            auto& trails = config->projectiles[currentItem].trails;

            ImGui::Checkbox("Trails", &trails.enabled);
            ImGui::SameLine(spacing + 77.0f);
            ImGui::PushID("Trails");

            if (ImGui::Button("..."))
                ImGui::OpenPopup("");

            if (ImGui::BeginPopup("")) {
                constexpr auto trailPicker = [](const char* name, Trail& trail) noexcept {
                    ImGui::PushID(name);
                    ImGuiCustom::colorPicker(name, trail);
                    ImGui::SameLine(150.0f);
                    ImGui::SetNextItemWidth(95.0f);
                    ImGui::Combo("", &trail.type, "Line\0Circles\0Filled Circles\0");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(95.0f);
                    ImGui::InputFloat("Time", &trail.time, 0.1f, 0.5f, "%.1fs");
                    trail.time = std::clamp(trail.time, 1.0f, 60.0f);
                    ImGui::PopID();
                };

                trailPicker("Local Player", trails.localPlayer);
                trailPicker("Allies", trails.allies);
                trailPicker("Enemies", trails.enemies);
                ImGui::EndPopup();
            }

            ImGui::PopID();
        }

        ImGui::SetNextItemWidth(95.0f);
        ImGui::InputFloat("Text Cull Distance", &sharedConfig.textCullDistance, 0.4f, 0.8f, "%.1fm");
        sharedConfig.textCullDistance = std::clamp(sharedConfig.textCullDistance, 0.0f, 999.9f);
    }

    ImGui::EndChild();
}
