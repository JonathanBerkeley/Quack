#include "pch.hpp"
#include "flashpoint.hpp"


/**
 * \brief Entry point for the application
 * \param self_module Handle to DLL module
 * \param call_reason Reason for calling function
 * \param lp_reserved Reserved
 * \return Always returns true
 */
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
