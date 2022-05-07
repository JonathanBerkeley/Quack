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


/**
 * \brief This is a global variable as it's used in std::atexit, which takes no arguments
 * It is used only in this file
 */
static PROCESS_INFORMATION ac_client{};


/**
 * \brief Starts the anti-cheat executable
 */
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


/**
 * \brief Kills the client anti-cheat that was started by InitClientAC
 */
void KillClientAC() {
    TerminateProcess(ac_client.hProcess, 0);
    CloseHandle(ac_client.hProcess);
    CloseHandle(ac_client.hThread);
}


/**
 * \brief Entry point and setup
 * \param lpParam Optional parameter passed from DllMain
 * \return Does not return
 */
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

    // Start standalone anti-cheat client
    InitClientAC();

    // On exit, run KillClientAC
    std::atexit(KillClientAC);

    // Main program loop
    TaskDispatch();
}
