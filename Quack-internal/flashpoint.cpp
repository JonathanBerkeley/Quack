#include "pch.h"
#include "flashpoint.h"
#include "constants.h"
#include "data.h"

#include <vector>
#include <chrono>
#include <thread>
#include <iomanip>

namespace thread = std::this_thread;
using ull = unsigned long long;

using namespace std::chrono_literals;
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
    // Redirect stdout & stderr to new console
    AllocConsole();
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
    freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);

    // Set title to console and output info on DLL
    SetConsoleTitle(constants::DLL_NAME);
    std::wcout << constants::DLL_NAME << L" loaded" << '\n';
    std::cout << "Version - " << constants::VERSION << '\n';

    InitClientAC();
    LogicLoop();

    // ReSharper disable once CppZeroConstantCanBeReplacedWithNullptr
    return 0;
}

void LogicLoop() {
    const auto this_module = GetModuleHandle(nullptr);
    while (running) {
        // todo: Detection logic
        // todo: Signature scanning
        // todo: internal heartbeat

        thread::sleep_for(1'000ms);
    }
}
