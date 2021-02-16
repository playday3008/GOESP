#include "Hooks.h"

#ifdef _WIN32

#include <Windows.h>

extern "C" BOOL WINAPI _CRT_INIT(HMODULE moduleHandle, DWORD reason, LPVOID reserved);

HMODULE hDll;

DWORD WINAPI OnDllAttach(LPVOID lpParameter)
{
    while (GetModuleHandle(L"serverbrowser.dll") == nullptr)
        Sleep(250);

    hooks = std::make_unique<Hooks>(hDll);
    hooks->setup();

    return EXIT_SUCCESS;
};

BOOL APIENTRY DllEntryPoint(HMODULE moduleHandle, DWORD reason, LPVOID reserved)
{
    if (!_CRT_INIT(moduleHandle, reason, reserved))
        return FALSE;

    if (reason == DLL_PROCESS_ATTACH) {
        hDll = moduleHandle;
        DisableThreadLibraryCalls(moduleHandle);
        detachedThread = CreateThread(nullptr, 0UL, OnDllAttach, moduleHandle, 0UL, nullptr);
    }

    return TRUE;
}

#else

void __attribute__((constructor)) DllEntryPoint()
{
    hooks = std::make_unique<Hooks>();
    hooks->setup();
}

#endif
