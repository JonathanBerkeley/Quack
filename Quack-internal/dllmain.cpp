#include "pch.h"
#include "flashpoint.h"
#include "data.h"

// Entry point for the application
BOOL APIENTRY DllMain(const HMODULE self_module, const DWORD call_reason, LPVOID lp_reserved) {
    if (call_reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(self_module);

        // ReSharper disable once CppLocalVariableMayBeConst
        // Create thread in host application
        HANDLE thread = CreateThread(
            nullptr,
            0,
            Init,
            self_module,
            0,
            nullptr
        );

        data::proc::self_module = self_module;

        if (thread)
            CloseHandle(thread);
    }
    return TRUE;
}
