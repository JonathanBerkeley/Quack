#include "pch.h"

#include "memory_scanner.h"

#include <SoftPub.h>
#include <WinTrust.h>
#include <wincrypt.h>

// Link with the WinTrust.lib file
#pragma comment (lib, "wintrust")

// Enumerates through linked modules, returns vector of results
std::vector<HMODULE> EnumerateModules(const ProcessInfo pi) {
    HMODULE modules[0x400];
    DWORD cbNeeded;

    std::vector<HMODULE> dlls;

    if (EnumProcessModules(pi.hProcess, modules, sizeof(modules), &cbNeeded))
        for (auto i = 0u; i < cbNeeded / sizeof(HMODULE); ++i)
            dlls.push_back(modules[i]);

    return dlls;
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


// Extracted from CSGOSimple with minor changes
// https://github.com/spirthack/CSGOSimple
std::uint8_t* PatternScan(HMODULE module, const char* signature) {
    static auto pattern_to_byte = [](const char* pattern) {
        auto bytes = std::vector<int>{};
        const auto start = const_cast<char*>(pattern);
        const auto end = const_cast<char*>(pattern) + std::strlen(pattern);

        for (auto current = start; current < end; ++current) {
            if (*current == '?') {
                ++current;
                if (*current == '?')
                    ++current;
                bytes.push_back(-1);
            }
            else {
                bytes.push_back(std::strtoul(current, &current, 16));
            }
        }
        return bytes;
    };

    const auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(module);
    const auto e_lfanew = reinterpret_cast<std::uint8_t*>(module) + dos_header->e_lfanew;
    const auto nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(e_lfanew);

    const auto size_of_image = nt_headers->OptionalHeader.SizeOfImage;
    const auto pattern_bytes = pattern_to_byte(signature);
    const auto scan_bytes = reinterpret_cast<std::uint8_t*>(module);

    const auto size = pattern_bytes.size();
    const auto data = pattern_bytes.data();

    for (auto i = 0ul; i < size_of_image - size; ++i) {
        bool found = true;
        for (auto j = 0ul; j < size; ++j) {
            if (scan_bytes[i + j] != data[j] && data[j] != -1) {
                found = false;
                break;
            }
        }
        if (found)
            return &scan_bytes[i];
    }
    return nullptr;
}
