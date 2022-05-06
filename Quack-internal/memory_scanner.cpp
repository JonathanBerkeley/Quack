// ReSharper disable CppClangTidyPerformanceNoIntToPtr
#include "pch.hpp"

#include "utils.hpp"
#include "memory_scanner.hpp"

#include <SoftPub.h>
#include <WinTrust.h>

#include "ban.hpp"
#include "hardware_id.hpp"

// Link with the WinTrust.lib file
#pragma comment (lib, "wintrust")

using namespace data;
using namespace std::string_literals;


#pragma region Modules
// Enumerates through linked modules, returns vector of results
std::vector<HMODULE> EnumerateModules(const ProcessInfo& pi) {
    HMODULE modules[0x400];
    DWORD cbNeeded;

    std::vector<HMODULE> dlls;

    if (EnumProcessModules(pi.handle, modules, sizeof(modules), &cbNeeded))
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
#pragma endregion

#pragma region Patterns
/**
 * \brief Converts supplied signature to bytes, removing wildcards
 * \param signature Signature to be converted to bytes
 * \return Vector of unsigned long, representing bytes
 */
std::vector<unsigned long> PatternToByte(const std::string& signature) {
    // Pattern of bytes to be returned
    std::vector<unsigned long> bytes{};

    // Pointer to the start of the signature
    const auto c_signature = const_cast<char*>(signature.c_str());

    // Start and end of pattern
    const auto start = c_signature;
    const auto end = c_signature + signature.length();

    // Loop over pattern
    for (char* byte = start; byte < end; ++byte) {

        // Check if signature has wildcard at this position
        if (*byte == '*') {
            ++byte;
            if (*byte == '*')
                ++byte;

            // Flag value pushed back
            bytes.push_back(0xDEADBEEF);
        }
        else {
            // Store as hex
            bytes.push_back(std::strtoul(byte, &byte, 16));
        }
    }

    return bytes;
}


/**
 * \brief Scans module for supplied signature
 *
 * Logic learned from example in CSGOSimple
 * https://github.com/spirthack/CSGOSimple
 * \param module Handle to module
 * \param signature The signature to be scanned for
 * \return If signature detected, a pointer to detected signature otherwise nullptr
 */
std::uint8_t* PatternScan(const HMODULE module, const std::string& signature) {
    const auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(module);
    const auto e_lfanew = reinterpret_cast<std::uint8_t*>(module) + dos_header->e_lfanew;
    const auto nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(e_lfanew);

    const auto size_of_image = nt_headers->OptionalHeader.SizeOfImage;
    const auto pattern_bytes = PatternToByte(signature);
    const auto scan_bytes = reinterpret_cast<std::uint8_t*>(module);

    const auto size = pattern_bytes.size();
    const auto data = pattern_bytes.data();

    for (auto i = 0ul; i < size_of_image - size; ++i) {
        bool found = true;
        for (auto j = 0ul; j < size; ++j) {
            if (scan_bytes[i + j] != data[j] && data[j] != 0xDEADBEEF) {
                found = false;
                break;
            }
        }
        if (found)
            return &scan_bytes[i];
    }
    return nullptr;
}


std::uint8_t* PatternScan(const std::string& signature, const MemoryRegion& mem_region) {
    const auto pattern_bytes = PatternToByte(signature);
    const auto size = pattern_bytes.size();
    const auto data = pattern_bytes.data();

    for (unsigned i = 0u; i < mem_region.size; ++i) {
        bool found = true;
        for (unsigned j = 0u; j < size; j++) {
            if (mem_region.start_address[i + j] != data[j] && data[j] != 0xDEADBEEF) {
                found = false;
                break;
            }
        }
        if (found)
            return &mem_region.start_address[i];
    }
    return nullptr;
}


// todo: Network this
Signatures GetSignatures(const ProcessInfo& context) {
    Signatures signatures{};

    // context->SendData();
    
    signatures.cheats.emplace_back(std::make_pair("Highlight"s, std::vector<std::string>{
        "50 00 72 00 6F 00 63 00 65 00 73 00 73 00 20 00 68 00 69 00 6A 00 61 00 63 00 6B 00 65 00 64"
    }));

    return signatures;
}
#pragma endregion

#pragma region Scans
void ModuleScan(const ProcessInfo& context, const bool unsigned_only) {

    auto [cheats] = GetSignatures(context);

    Log("\nBeginning module memory scan...\n");

    // ReSharper disable once CppLocalVariableMayBeConst
    auto dlls = EnumerateModules(context);

    // For each DLL in the process
    for (const auto& dll : dlls) {

        WCHAR module_path[MAX_PATH];
        static constinit int size = sizeof(module_path) / sizeof(TCHAR);

        if (!GetModuleFileNameExW(context.handle, dll, module_path, size))
            return;

        auto scan = [&cheats, &dll, &module_path, &context] {

            if (dll == context.this_module)
                return;

            // For every cheat
            for (const auto& [cheat_name, signatures] : cheats) {

                // Try every known pattern
                for (const auto& pattern : signatures) {

                    // Check for pattern match
                    if (const auto addr = PatternScan(dll, pattern); addr) {

                        DetectionInfo detection_info{
                            .cheat_name = cheat_name,
                            .scan_type = "Module scan",
                            .cheat_path = module_path
                        };
                        Ban(detection_info);
                    }

                }

            }
        };

        if (unsigned_only) {
            if (!VerifyModule(module_path)) {
                scan();
                Log(module_path);
            }
        }
        else {
            scan();
            Log(module_path);
        }
    }

    Log("\nFinished module memory scan...\n");
}


void RegionScan(const ProcessInfo& context, const MemoryRegion& memory_region) {

    auto [cheats] = GetSignatures(context);

    // Log("\nBeginning executable memory scan...\n");

    for (const auto& [cheat_name, signatures] : cheats) {

        for (const auto& pattern : signatures) {

            if (const auto addr = PatternScan(pattern, memory_region); addr) {
                DetectionInfo detection_info{
                    .cheat_name = cheat_name,
                    .scan_type = "RegionScan",
                    .cheat_path = L"Mapped cheat"
                };
                Ban(detection_info);
            }

        }
    }

    // Log("\nFinished executable memory scan...\n");
}


void FullScan(const ProcessInfo& context) {
    const std::vector<DWORD> mask{ PAGE_EXECUTE, PAGE_EXECUTE_READWRITE, PAGE_EXECUTE_READ, PAGE_EXECUTE_WRITECOPY };

    MEMORY_BASIC_INFORMATION mbi{};
    LPVOID offset = nullptr;

    while (VirtualQueryEx(GetCurrentProcess(), offset, &mbi, sizeof mbi)) {

        // Move the next scan onto the next region by offsetting by the current region's size
        const auto region = reinterpret_cast<DWORD_PTR>(mbi.BaseAddress) + mbi.RegionSize;
        offset = reinterpret_cast<LPVOID>(region);

        // Check if this memory region is executable
        if (std::ranges::find(mask, mbi.AllocationProtect) != mask.end()) {

            MemoryRegion memory_region{
                .size = mbi.RegionSize,
                .start_address = static_cast<std::uint8_t*>(mbi.BaseAddress)
            };

            RegionScan(context, memory_region);
        }
    }
}
#pragma endregion
