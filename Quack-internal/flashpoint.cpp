#include "pch.h"
#include "flashpoint.h"
#include "constants.h"
#include "data.h"

#include <vector>

using namespace data;

DWORD WINAPI Init(LPVOID lpParam) {
    // Redirect stdout & stderr to new console
    AllocConsole();
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
    freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);

    // Set title to console and output info on DLL
    SetConsoleTitle(constants::DLL_NAME);
    std::wcout << constants::DLL_NAME << L" injected" << '\n';
    std::cout << "Version - " << constants::VERSION << '\n';

    LogicLoop();

    // ReSharper disable once CppZeroConstantCanBeReplacedWithNullptr
    return 0;
}

void LogicLoop() {
    while (running) {

        Sleep(300);
        // Todo: internal heartbeat
    }
}