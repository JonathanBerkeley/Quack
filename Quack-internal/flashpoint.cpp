#include "pch.h"
#include "flashpoint.h"
#include "constants.h"
#include "data.h"

#include <chrono>
#include <thread>
#include <iomanip>

#include "memory_scanner.h"

namespace thread = std::this_thread;
using ull = unsigned long long;

using namespace std::chrono_literals;
using constants::DBG;
using namespace data;

void InitClientAC() {
    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};

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
        &pi
    );

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
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
            TCHAR mod_name[MAX_PATH];
            static constinit int size = sizeof(mod_name) / sizeof(TCHAR);

            if (GetModuleFileNameEx(process_info.hProcess, dll, mod_name, size)) {
                if constexpr (DBG)
                    std::wcout << "Module:\t" << mod_name << "\n";
            }
        }
        
        if constexpr (DBG)
            std::cout << "\n\n\n";
        thread::sleep_for(10s);
    }
}
