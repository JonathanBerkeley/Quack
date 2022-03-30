#include "pch.h"
#include "flashpoint.hpp"
#include "constants.hpp"
#include "data.hpp"
#include "dns_walk.hpp"
#include "config.hpp"
#include "network.h"
#include "memory_scanner.h"

namespace json = nlohmann;

namespace thread = std::this_thread;

using namespace std::chrono_literals;
using namespace cfg;
using namespace data;
using constants::DBG;
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
        SetConsoleTitle(constants::W_DLL_NAME);
        std::cout << constants::DLL_NAME << " loaded" << '\n';
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

    const ProcessInfo process_info{
        GetCurrentProcess(),
        GetCurrentProcessId(),
        GetModuleHandle(nullptr)
    };

    const json::json heartbeat{
        {"Heartbeat", true}
    };


    if constexpr (DNSScanning) {
        if (const auto entries = CheckForBlacklistedDNSEntries()) {
            std::cout << "Blacklisted domain(s) found: \n";

            for (const auto& [name, type] : entries.value()) {
                std::wcout << name << L' ' << type << '\n';
            }
        }
    }


    while (running) {
        // todo: Detection logic
        // todo: Signature scanning
        // todo: Internal heartbeat

        Communication::SendData(heartbeat);

        if constexpr (SignatureScanning)
            // Scan the modules in memory of target process
            ModuleScan(process_info, UnsignedModulesOnly);

        thread::sleep_for(10s);
    }
}
