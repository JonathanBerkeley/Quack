// ReSharper disable CppClangTidyPerformanceNoIntToPtr
#include "pch.hpp"

#include "utils.hpp"
#include "ban.hpp"
#include "memory_scanner.hpp"

// 3rd party
#include <SoftPub.h>
#include <WinTrust.h>

#include "network.hpp"

// Link with the WinTrust.lib file
#pragma comment (lib, "wintrust")

using namespace data;
using namespace std::string_literals;


/**
 * \brief Check if a pointer is valid
 *
 * Adapted from: https://guidedhacking.com/threads/testing-if-pointer-is-invalid.13222/#post-77709
 * \param ptr Pointer to check
 * \return True if pointer is valid
 */
bool PointerIsValid(const void* ptr) {
    MEMORY_BASIC_INFORMATION mbi{};

    if (VirtualQuery(ptr, &mbi, sizeof mbi)) {
        constexpr auto mask = PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
        bool protect = mbi.Protect & mask;

        if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))
            protect = false;

        return protect;
    }

    return false;
}


#pragma region Modules
/**
 * \brief Enumerates through loaded modules, returns vector of results
 * \param process_info Information about the current process
 * \return Vector of HMODULEs that correspond to the starting address of loaded modules
 */
std::vector<HMODULE> EnumerateModules(const ProcessInfo& process_info) {
    HMODULE modules[0x400];
    DWORD cbNeeded;

    std::vector<HMODULE> dlls;

    if (EnumProcessModules(process_info.handle, modules, sizeof(modules), &cbNeeded))
        for (auto i = 0u; i < cbNeeded / sizeof(HMODULE); ++i)
            dlls.push_back(modules[i]);

    return dlls;
}


/**
 * \brief Verifies signature for source file, based on MSDN example code
 *
 * https://docs.microsoft.com/en-us/windows/win32/seccrypto/example-c-program--verifying-the-signature-of-a-pe-file
 * \param source_file Image name to verify
 * \return True if the module is verified, false otherwise
 */
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


/**
 * \brief Scans a region for a provided signature
 * \param signature Signature to search for
 * \param mem_region Memory region to search
 * \return Pointer to start address of matched pattern
 */
std::uint8_t* PatternScan(const std::string& signature, const MemoryRegion& mem_region) {

    if (not PointerIsValid(mem_region.start_address))
        return nullptr;

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


/**
 * \brief Fetches known cheat signatures from local storage
 * \return Signature data
 */
Signatures GetLocalSignatures() {
    Signatures signatures{};

    signatures.cheats.emplace_back(std::make_pair("Highlight"s, std::vector<std::string>{
        "50 00 72 00 6F 00 63 00 65 00 73 00 73 00 20 00 68 00 69 00 6A 00 61 00 63 00 6B 00 65 00 64"
    }));

    signatures.cheats.emplace_back(std::make_pair("Inertia-Cheat"s, std::vector<std::string>{
        "55 8B EC 83 EC 2C A1 * * * * 33 C5 89 45 FC 53 56 8B 35 * * * * 57 6A 23 8B F9 FF D6 A8 01 0F 85",
            "68 * * * * FF 15 * * * * 8B 35 * * * * 8B 3D * * * * 03 F0 A1 * * * * 89 45 C8 3B F8 74 5D",
            "50 A1 * * * * 33 C5 50 8D 45 F4 64 A3 * * * * 6A 19"
    }));

    return signatures;
}


/**
 * \brief Fetches known cheat signatures
 *
 * \param context Information about the current process
 * \return Signature data
 */
Signatures GetSignatures(const ProcessInfo& context) {
    // todo: fix networking problems
    static Signatures signatures{};

    if (not signatures.cheats.empty())
        return signatures;
    
    signatures = Communication::GetSignatures();
    return signatures;
}
#pragma endregion


#pragma region Scans
/**
 * \brief Scans a module that is loaded into the process
 * \param context Information about the current process
 * \param unsigned_only Whether to scan unsigned modules only, or to scan all modules
 */
void ModuleScan(const ProcessInfo& context, const bool unsigned_only) {

    auto [cheats] = GetLocalSignatures();

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
                    if (PatternScan(dll, pattern)) {

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


/**
 * \brief Scans a region of memory provided by param memory_region
 * \param context Information about the current process
 * \param memory_region Information about the memory region to scan, containing size and start address
 */
void RegionScan(const ProcessInfo& context, const MemoryRegion& memory_region) {

    auto [cheats] = GetLocalSignatures();

    // Log("\nBeginning executable memory scan...\n");

    for (const auto& [cheat_name, signatures] : cheats) {

        for (const auto& pattern : signatures) {

            if (PatternScan(pattern, memory_region)) {
                DetectionInfo detection_info{
                    .cheat_name = cheat_name,
                    .scan_type = "Region Scan",
                    .cheat_path = L"0"
                };
                Ban(detection_info);
            }

        }
    }

    // Log("\nFinished executable memory scan...\n");
}


/**
 * \brief Scan all of executable memory
 * \param context Information about the current process
 */
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
