#include "pch.hpp"
#include "flashpoint.hpp"
#include "constants.hpp"
#include "data.hpp"
#include "config.hpp"
#include "task_dispatch.hpp"
#include "utils.hpp"

using namespace std::chrono_literals;
using namespace cfg;
using namespace data;
using constants::DBG;

// This is a global variable as it's used in std::atexit, which takes no arguments
// It is used only in this file
static PROCESS_INFORMATION ac_client{};


// Start the anti-cheat executable
void InitClientAC() {
    STARTUPINFOA si{};

#pragma warning( suppress : 6335 )
    CreateProcessA(
        "Quack-ac.exe",
        nullptr,
        nullptr,
        nullptr,
        FALSE,
        CREATE_NEW_CONSOLE,
        nullptr,
        nullptr,
        &si,
        &ac_client
    );
}


void KillClientAC() {
    TerminateProcess(ac_client.hProcess, 0);
    CloseHandle(ac_client.hProcess);
    CloseHandle(ac_client.hThread);
}


// Entry point and setup
DWORD WINAPI Init(LPVOID lpParam) {

    if constexpr (DBG) {
        // Redirect stdout & stderr to new console
        AllocConsole();
        freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
        freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);

        // Set title to console and output info on DLL
        SetConsoleTitle(constants::W_DLL_NAME);
        std::cout << constants::DLL_NAME << " loaded" << '\n';
        std::cout << "Version - " << constants::VERSION << '\n';
    }

    InitClientAC();

    // On exit, run KillClientAC
    std::atexit(KillClientAC);

    TaskDispatch();

    // ReSharper disable once CppZeroConstantCanBeReplacedWithNullptr
    return 0;
}
