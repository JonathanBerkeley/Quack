// ReSharper disable CppClangTidyClangDiagnosticFourCharConstants
#include "pch.hpp"
#include "hardware_id.hpp"
#include "config.hpp"

// 3rd party
#include "smbios.hpp"




#pragma region Public
HardwareID::HardwareID() : raw_hwid(GetHWID().value_or("")) {
    hash(raw_hwid);
}

std::string HardwareID::GetHash() {
    return hash.getHash();
}

std::string HardwareID::GetRawHWID() {
    return raw_hwid;
}
#pragma endregion

#pragma region Private
/**
 * \brief Retrieves hardware ID based on config settings
 * \return std::string encoded representation of hardware ID on success, std::nullopt otherwise
 */
std::optional<std::string> HardwareID::GetHWID() {
    // Retrieve size needed for table
    const auto smbios_size = GetSystemFirmwareTable(
        'RSMB',
        0,
        nullptr,
        0
    );

    // Allocate room for the table
    const auto smbios_data = static_cast<smbios::RawSMBIOSData*>(HeapAlloc(GetProcessHeap(), 0, smbios_size));
    if (not smbios_data)
        return {};

    // Retrieve SMBIOS table
    const auto bytes_written = GetSystemFirmwareTable('RSMB', 0, smbios_data, smbios_size);
    if (not bytes_written)
        return {};

    const auto smbios_table = smbios_data->SMBIOSTableData;

    // Create HWID
    std::stringstream hwid;
    hwid << static_cast<int>(smbios_table[0x4]) << static_cast<int>(smbios_table[0x5]) << static_cast<int>(smbios_table[0x8]);

    if constexpr (cfg::Features::GpuID)
        hwid << GpuID().value_or("");

    if constexpr (cfg::Features::DiskID)
        hwid << DiskID().value_or(0);

    if constexpr (cfg::Features::CpuID)
        hwid << static_cast<int>(GetCpuID());

    // Cleanup
    HeapFree(GetProcessHeap(), 0, smbios_data);

    return hwid.str();
}

/**
 * \brief Retrieve serial information from main computer storage
 * \return Serial on success, std::nullopt otherwise
 */
std::optional<DWORD> HardwareID::DiskID() {
    DWORD serial{};
    if (not GetVolumeInformationA(
        "C://",
        nullptr,
        0,
        &serial,
        nullptr,
        nullptr,
        nullptr,
        0))
        return {};

    return serial;
}

/**
 * \brief Retrieves hardware ID of GPU
 * \return std::string encoded representation of GPU serials on success, std::nullopt on failure
 */
std::optional<std::string> HardwareID::GpuID() {
    IDirect3D9* d9_object = Direct3DCreate9(D3D_SDK_VERSION);
    if (not d9_object)
        return {};

    const auto adapter_count = d9_object->GetAdapterCount();
    const auto adapters = new D3DADAPTER_IDENTIFIER9[sizeof adapter_count]{};

    for (auto i = 0u; i < adapter_count; i++)
        d9_object->GetAdapterIdentifier(i, 0, &adapters[i]);

    std::stringstream ss;
    const auto [Data1, Data2, Data3, Data4] = adapters->DeviceIdentifier;
    ss << Data1 << Data2 << Data3;

    for (int i = 0; i < 7; ++i)
        ss << static_cast<short>(Data4[i]);

    // Cleanup
    delete[] adapters;

    return ss.str();
}

/**
 * \brief Retrieve CPU ID through intrinsic function __cpuid()
 * \return chat16_t representation of CPU ID
 */
char16_t HardwareID::GetCpuID() {
    // EAX, EBX, ECX, EDX
    int cpu_id[4] = { 0, 0, 0, 0 };
    __cpuid(cpu_id, 0);

    char16_t id_output = 0;
    const auto* ptr = reinterpret_cast<char16_t*>(cpu_id);

    for (char32_t i = 0; i < 8; ++i)
        id_output += ptr[i];

    return id_output;
}
#pragma endregion
