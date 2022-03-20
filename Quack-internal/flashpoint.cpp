#include "pch.h"
#include "flashpoint.h"
#include "constants.h"
#include "data.h"
#include "network.h"
#include "memory_scanner.h"

#include "Lib/json.hpp"

namespace json = nlohmann;

namespace thread = std::this_thread;

using namespace std::chrono_literals;
using constants::DBG;
using namespace data;
using ull = unsigned long long;

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


DWORD WINAPI Init(LPVOID lpParam) {
    
    if constexpr (DBG) {
        // Redirect stdout & stderr to new console
        AllocConsole();
        freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
        freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);

        // Set title to console and output info on DLL
        SetConsoleTitle(constants::DLL_NAME);
        std::wcout << constants::DLL_NAME << L" loaded" << '\n';
        std::cout << "Version - " << constants::VERSION << '\n';
    }

    InitClientAC();

    // On exit, run KillClientAC
    std::atexit(KillClientAC);

    LogicLoop();

    // ReSharper disable once CppZeroConstantCanBeReplacedWithNullptr
    return 0;
}


void LogicLoop() {
    Communication network{};

    const ProcessInfo process_info{
        GetCurrentProcess(),
        GetCurrentProcessId(),
        GetModuleHandle(nullptr),
        &network
    };

    const json::json heartbeat{
        {"Heartbeat", true}
    };

    while (running) {
        // todo: Detection logic
        // todo: Signature scanning
        // todo: Internal heartbeat
        
        network.SendData(heartbeat);

        // Scan the modules in memory of target process
        ModuleScan(process_info, true);

        thread::sleep_for(10s);
    }
}
