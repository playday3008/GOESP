#include <memory>

#include "Hooks.h"

#ifdef _WIN32

#include "SDK/Platform.h"

#include <Windows.h>
#include "AntiDetection.h"

extern "C" BOOL WINAPI _CRT_INIT(HMODULE moduleHandle, DWORD reason, LPVOID reserved);

DWORD WINAPI OnDllAttach(LPVOID lpParameter)
{
    while (GetModuleHandleA(SERVERBROWSER_DLL) == nullptr)
        Sleep(250);

    hooks = std::make_unique<Hooks>(static_cast<HMODULE>(lpParameter));
    hooks->setup();

    return EXIT_SUCCESS;
};

BOOL APIENTRY DllEntryPoint(HMODULE moduleHandle, DWORD reason, LPVOID reserved)
{
    if (!_CRT_INIT(moduleHandle, reason, reserved))
        return FALSE;

    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(moduleHandle);
    	
        if (AntiDetection::RemovePeHeader(moduleHandle) && IsDebuggerPresent())
            OutputDebugStringA("CleanUp PE Header Success.\n");
    	
        if (AntiDetection::UnlinkModule(moduleHandle) && IsDebuggerPresent())
            OutputDebugStringA("Unlink module from PEB success.\n");
    	
        CreateThread(nullptr, NULL, OnDllAttach, moduleHandle, NULL, nullptr);
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
