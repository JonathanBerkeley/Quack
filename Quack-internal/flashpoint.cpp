#include "pch.h"
#include "flashpoint.h"
#include "constants.h"
#include "data.h"

#include <chrono>
#include <thread>
#include <iomanip>
#include <Softpub.h>
#include <wincrypt.h>
#include <WinTrust.h>

#include "memory_scanner.h"

// Link with the WinTrust.lib file
#pragma comment (lib, "wintrust")

namespace thread = std::this_thread;
using ull = unsigned long long;

using namespace std::chrono_literals;
using constants::DBG;
using namespace data;

// Start the anti-cheat executable
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


// Verifies signature for source file, based on MSDN example code
// https://docs.microsoft.com/en-us/windows/win32/seccrypto/example-c-program--verifying-the-signature-of-a-pe-file
bool VerifyModule(const LPCWSTR source_file) {
    WINTRUST_FILE_INFO file_data{};
    file_data.cbStruct = sizeof(WINTRUST_FILE_INFO);
    file_data.pcwszFilePath = source_file;

    GUID policy_guid = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    WINTRUST_DATA win_trust_data{
        .cbStruct = sizeof(win_trust_data),
        .pPolicyCallbackData = nullptr,          // Use default code signing EKU
        .pSIPClientData = nullptr,               // Not applicable
        .dwUIChoice = WTD_UI_NONE,               // Disable WVT UI
        .fdwRevocationChecks = WTD_REVOKE_NONE,  // No revocation checking.
        .dwUnionChoice = WTD_CHOICE_FILE,        // Verify an embedded signature on a file
        .pFile = &file_data,                     // Setup pointer to file data
        .dwStateAction = WTD_STATEACTION_VERIFY, // Verify action
        .hWVTStateData = nullptr,                // Verification sets this value
        .dwUIContext = 0                         // Not applicable with no UI
    };

    bool verified = false;

    // WinVerifyTrust verifies signatures as specified by the GUID and win_trust_data
    if (WinVerifyTrust(nullptr, &policy_guid, &win_trust_data) == ERROR_SUCCESS) {
        verified = true;
    }

    // Any hWVTStateData must be released by a call with close.
    win_trust_data.dwStateAction = WTD_STATEACTION_CLOSE;
    return verified;
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
                    std::wcout << "Module:\t" << mod_name << " VERIFIED: " << (VerifyModule(mod_name) ? "True" : "False") << "\n";
            }
        }

        if constexpr (DBG)
            std::cout << "\n\n\n";
        thread::sleep_for(10s);
    }
}
