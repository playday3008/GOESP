#include "Hooks.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

#include "Config.h"
#include "EventListener.h"
#include "GUI.h"
#include "Hacks/ESP.h"
#include "Hacks/Misc.h"
#include "Interfaces.h"
#include "Memory.h"

#include "SDK/Engine.h"
#include "SDK/GlobalVars.h"
#include "SDK/InputSystem.h"

#include <atomic>
#include <intrin.h>

class HookGuard {
public:
    HookGuard() { ++atomic; }
    ~HookGuard() { --atomic; }
    static bool freed() { return atomic == 0; }
private:
    inline static std::atomic_int atomic;
};

static LRESULT WINAPI wndProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    HookGuard guard;

    static const auto once = [](HWND window) noexcept {
        eventListener = std::make_unique<EventListener>();
        config = std::make_unique<Config>("GOESP");

        ImGui::CreateContext();
        ImGui_ImplWin32_Init(window);
        gui = std::make_unique<GUI>();

        hooks->install();

        return true;
    }(window);

    static auto lastDataGather = 0.0f;

    if (lastDataGather != memory->globalVars->realtime) {
        lastDataGather = memory->globalVars->realtime;

        ESP::collectData();
        Misc::collectData();
    }

    LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    ImGui_ImplWin32_WndProcHandler(window, msg, wParam, lParam);
    interfaces->inputSystem->enableInput(!gui->open);

    return CallWindowProc(hooks->wndProc, window, msg, wParam, lParam);
}

static HRESULT D3DAPI reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params) noexcept
{
    HookGuard guard;

    ImGui_ImplDX9_InvalidateDeviceObjects();
    return hooks->reset(device, params);
}

static HRESULT D3DAPI present(IDirect3DDevice9* device, const RECT* src, const RECT* dest, HWND windowOverride, const RGNDATA* dirtyRegion) noexcept
{
    HookGuard guard;

    static const auto _ = ImGui_ImplDX9_Init(device);

    IDirect3DVertexDeclaration9* vertexDeclaration;
    device->GetVertexDeclaration(&vertexDeclaration);

    if (config->loadScheduledFonts())
        ImGui_ImplDX9_InvalidateDeviceObjects();

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ESP::render();
    Misc::drawReloadProgress(ImGui::GetBackgroundDrawList());
    Misc::drawRecoilCrosshair(ImGui::GetBackgroundDrawList());
    Misc::purchaseList();
    Misc::drawBombZoneHint();

    gui->render();

    if (ImGui::IsKeyPressed(VK_INSERT, false)) {
        gui->open = !gui->open;
        if (!gui->open)
            interfaces->inputSystem->resetInputState();
    }
    ImGui::GetIO().MouseDrawCursor = gui->open;

    ImGui::EndFrame();

    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    device->SetVertexDeclaration(vertexDeclaration);
    vertexDeclaration->Release();

    return hooks->present(device, src, dest, windowOverride, dirtyRegion);
}

static BOOL WINAPI setCursorPos(int X, int Y) noexcept
{
    HookGuard guard;

    return gui->open || hooks->setCursorPos(X, Y);
}

Hooks::Hooks(HMODULE module) noexcept
{
    this->module = module;

    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

    // interfaces and memory shouldn't be initialized in wndProc because they show MessageBox on error which would cause deadlock
    interfaces = std::make_unique<const Interfaces>();
    memory = std::make_unique<const Memory>();

    window = FindWindowW(L"Valve001", nullptr);
    wndProc = WNDPROC(SetWindowLongPtrA(window, GWLP_WNDPROC, LONG_PTR(::wndProc)));
}

void Hooks::install() noexcept
{
    assert(memory);

    reset = *reinterpret_cast<decltype(reset)*>(memory->reset);
    *reinterpret_cast<decltype(::reset)**>(memory->reset) = ::reset;

    present = *reinterpret_cast<decltype(present)*>(memory->present);
    *reinterpret_cast<decltype(::present)**>(memory->present) = ::present;

    setCursorPos = *reinterpret_cast<decltype(setCursorPos)*>(memory->setCursorPos);
    *reinterpret_cast<decltype(::setCursorPos)**>(memory->setCursorPos) = ::setCursorPos;
}

extern "C" BOOL WINAPI _CRT_INIT(HMODULE module, DWORD reason, LPVOID reserved);

static DWORD WINAPI waitOnUnload(HMODULE hModule) noexcept
{
    while (!HookGuard::freed())
        Sleep(50);

    interfaces->inputSystem->enableInput(true);
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    eventListener.reset();

    _CRT_INIT(hModule, DLL_PROCESS_DETACH, nullptr);
    FreeLibraryAndExitThread(hModule, 0);
}

void Hooks::uninstall() noexcept
{
    *reinterpret_cast<void**>(memory->reset) = reset;
    *reinterpret_cast<void**>(memory->present) = present;
    *reinterpret_cast<void**>(memory->setCursorPos) = setCursorPos;

    SetWindowLongPtrA(window, GWLP_WNDPROC, LONG_PTR(wndProc));

    if (HANDLE thread = CreateThread(nullptr, 0, LPTHREAD_START_ROUTINE(waitOnUnload), module, 0, nullptr))
        CloseHandle(thread);
}
