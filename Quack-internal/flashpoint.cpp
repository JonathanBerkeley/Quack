#include "pch.h"
#include "flashpoint.h"
#include "constants.h"
#include "data.h"

#include <chrono>
#include <thread>
#include <iomanip>
#include <algorithm>

#include "memory_scanner.h"
#include "Lib/httplib.hpp"
#include "Lib/json.hpp"

namespace http = httplib;
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
    const ProcessInfo process_info{
        GetCurrentProcess(),
        GetCurrentProcessId(),
        GetModuleHandle(nullptr)
    };

    http::Client cli{ "localhost", constants::IPC_PORT };

    const std::vector<std::string> cheat_patterns{
        "50 00 72 00 6F 00 63 00 65 00 73 00 73 00 20 00 68 00 69 00 6A 00 61 00 63 00 6B 00 65 00 64"
    };

    while (running) {
        // todo: Detection logic
        // todo: Signature scanning
        // todo: Internal heartbeat

        json::json body{};
        body["Test"] = {
            {"Hello", "World"}
        };

        if (auto res = cli.Post("/", body.dump(), "application/json")) {
            if (res->status == 200) {
                if constexpr (DBG) {
                    std::cout << res->body << std::endl;
                }
            }
            else {
                std::cout << "Failed: " << res->status;
                std::cout << "\nError:" << res.error();
            }
        }


        if (DBG)
            std::cout << "\nBeginning memory scan...\n";
        auto dlls = EnumerateModules(process_info);
        for (const auto& dll : dlls) {
            
            TCHAR module_path[MAX_PATH];
            static constinit int size = sizeof(module_path) / sizeof(TCHAR);

            if (GetModuleFileNameEx(process_info.hProcess, dll, module_path, size)) {
                /*if constexpr (DBG)
                    std::wcout << "Module:\t" << mod_name << " VERIFIED: " << (VerifyModule(mod_name) ? "True" : "False") << "\n";*/

                if (!VerifyModule(module_path) and dll != process_info.this_module) {
                    std::wstring w_path{ module_path };

                    // todo: Concrete module whitelist
                    for (const auto& pattern : cheat_patterns) {
                        if (const auto addr = PatternScan(dll, pattern.c_str()); addr != nullptr) {
                            std::cout << "\nCHEAT FOUND:\n";
                            std::wcout << module_path << " Signature match at " << addr << '\n';

                            // ExitProcess(0);
                        }
                    }
                }
            }
        }
        if (DBG)
            std::cout << "\nFinished memory scan...\n";
        thread::sleep_for(10s);
    }
}
