#include "pch.hpp"
#include "flashpoint.hpp"
#include "data.hpp"

// Entry point for the application
BOOL APIENTRY DllMain(const HMODULE self_module, const DWORD call_reason, LPVOID lp_reserved) {
    if (call_reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(self_module);

        // ReSharper disable once CppLocalVariableMayBeConst
        // Create thread in host application
        if (HANDLE thread = CreateThread(
            nullptr,
            0,
            Init,
            self_module,
            0,
            nullptr
        ))
            CloseHandle(thread);
    }
    return TRUE;
}
