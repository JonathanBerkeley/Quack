#include "pch.h"
#include "flashpoint.h"
#include "constants.h"
#include "data.h"

#include <chrono>
#include <thread>
#include <iomanip>
#include <algorithm>

#include "memory_scanner.h"

namespace thread = std::this_thread;
using ull = unsigned long long;

using namespace std::chrono_literals;
using constants::DBG;
using namespace data;

static PROCESS_INFORMATION ac_client{};

// Start the anti-cheat executable
void InitClientAC() {
    STARTUPINFOA si{};

#pragma warning( suppress : 6335 )
    CreateProcessA(
        "Quack-client.exe",
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
    const ProcessInfo process_info{
        GetCurrentProcess(),
        GetCurrentProcessId(),
        GetModuleHandle(nullptr)
    };

    while (running) {
        // todo: Detection logic
        // todo: Signature scanning
        // todo: Internal heartbeat


        auto dlls = EnumerateModules(process_info);
        for (const auto& dll : dlls) {
            TCHAR module_path[MAX_PATH];
            static constinit int size = sizeof(module_path) / sizeof(TCHAR);

            if (GetModuleFileNameEx(process_info.hProcess, dll, module_path, size)) {
                /*if constexpr (DBG)
                    std::wcout << "Module:\t" << mod_name << " VERIFIED: " << (VerifyModule(mod_name) ? "True" : "False") << "\n";*/

                if (!VerifyModule(module_path) and dll != process_info.this_module) {
                    // FreeLibrary(dll);
                    
                    std::wstring w_path{ module_path };

                    // todo: Concrete module whitelist
                    if (w_path.find(L"Quack") == std::wstring::npos) {
                        std::wcout << "Ejecting module:\t" << module_path << "\n";
                        FreeLibrary(dll);
                    }
                }
            }
        }

        if constexpr (DBG)
            std::cout << "\n\n\n";
        thread::sleep_for(10s);
    }
}
