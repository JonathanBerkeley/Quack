#include "pch.hpp"

#include "utility.hpp"
#include "identification.hpp"

// 3rd party
#include "smbios.hpp"
#include "hash-library/sha256.hpp"
#pragma comment(lib, "iphlpapi.lib")

#pragma region HardwareID
HardwareID::HardwareID(): raw_hwid(GetHWID().value_or("")) {
    hash(raw_hwid);
}


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
    hwid << static_cast<int>(smbios_table[0x4]) << static_cast<int>(smbios_table[0x5])
        << static_cast<int>(smbios_table[0x8]) << GpuID().value_or("")
        << DiskID().value_or(0) << static_cast<int>(GetCpuID());

    // Cleanup
    HeapFree(GetProcessHeap(), 0, smbios_data);

    return hwid.str();
}


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


std::string HardwareID::GetHash() {
    return hash.getHash();
}


std::string HardwareID::GetRawHWID() {
    return raw_hwid;
}
#pragma endregion


/**
 * \brief Enumerate over Address Resolution Protocol (ARP) entries to extract hashes of MAC address
 * This can be used to identify users that are ban-evading.
 * Multiple users using the same network could trigger a match though, so it should be used
 * as a suspicious indicator only.
 * \return Vector of SHA256 hashes on success, std::nullopt on failure 
 */
std::optional<Hashes> GetArpMacHashes() {

    Hashes hashes{};
    // Pointer to ARP table
    MIB_IPNET_TABLE2* arp; 

    const auto result = GetIpNetTable2(AF_UNSPEC, &arp);
    if (result != NO_ERROR && result != ERROR_NOT_FOUND) 
        return std::nullopt;

    // Blacklist of the beginning of placeholder mac addresses
    const std::vector blacklist{ 0x0, 0xff, 0x33, 0x01 };

    Log("Identification - ARP:");
    // Loop over all the entries in the ARP table
    for (DWORD i = 0; i < arp->NumEntries; ++i) {
        const auto row = arp->Table[i];

        // Check the start of the mac address against the blacklist of placeholder addresses
        const auto current_bit = static_cast<int>(row.PhysicalAddress[0]);
        if (std::ranges::find(blacklist, current_bit) != blacklist.end())
            continue;

        // Create a SHA256 hash of the address
        SHA256 sha256;
        std::stringstream ss{};

        for (unsigned long j = 0; j < row.PhysicalAddressLength; j++) {
            if (j == row.PhysicalAddressLength - 1)
                ss << std::hex << static_cast<int>(row.PhysicalAddress[j]);
            else
                ss << std::hex << static_cast<int>(row.PhysicalAddress[j]) << '-';
        }

        Log(ss.str());
        hashes.emplace_back(
            sha256(ss.str())
        );
    }

    return IfNotEmpty(hashes);
}


/* Older Windows hashing method
PBYTE Hash(BYTE data[]) {
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
    class HashException : std::exception {};

    BCRYPT_ALG_HANDLE hasher{};
    BCRYPT_HASH_HANDLE hHash{};

    DWORD cbHashObject{ 0 }, cbData{ 0 }, cbHash{ 0 };
    PBYTE pbHash{}, pbHashObject{};

    try {
        // Get algorithm provider handle
        if (!NT_SUCCESS(BCryptOpenAlgorithmProvider(
            &hasher,
            BCRYPT_SHA256_ALGORITHM,
            nullptr,
            BCRYPT_HASH_REUSABLE_FLAG
        ))) {
            throw HashException{};
        }

        // Calculate buffer size required
        if (!NT_SUCCESS(BCryptGetProperty(
            hasher,
            BCRYPT_OBJECT_LENGTH,
            reinterpret_cast<PBYTE>(&cbHashObject),
            sizeof(DWORD),
            &cbData,
            0
        ))) {
            throw HashException{};
        }

        // Allocate the hash object
        pbHashObject = static_cast<PBYTE>(HeapAlloc(GetProcessHeap(), 0, cbHashObject));
        if (pbHashObject == nullptr)
            throw HashException{};

        // Calculate length of hash
        if (!NT_SUCCESS(BCryptGetProperty(
            hasher,
            BCRYPT_HASH_LENGTH,
            reinterpret_cast<PBYTE>(&cbHash),
            sizeof(DWORD),
            &cbData,
            0
        ))) {
            throw HashException{};
        }

        // Allocate the hash buffer
        pbHash = static_cast<PBYTE>(HeapAlloc(GetProcessHeap(), 0, cbHash));
        if (pbHash == nullptr)
            throw HashException{};

        // Create hash
        if (!NT_SUCCESS(BCryptCreateHash(
            hasher,
            &hHash,
            pbHashObject,
            cbHashObject,
            nullptr,
            0,
            0
        ))) {
            throw HashException{};
        }

        // Hash data
        if (!NT_SUCCESS(BCryptHashData(
            hHash,
            data,
            sizeof(data),
            0
        ))) {
            throw HashException{};
        }

        if (!NT_SUCCESS(BCryptFinishHash(
            hHash,
            pbHash,
            cbHash,
            0
        ))) {
            throw HashException{};
        }

        return pbHash;
    }
    catch (const HashException&) {
        // Cleanup
        if (hasher)
            BCryptCloseAlgorithmProvider(hasher, 0);
        if (hHash)
            BCryptDestroyHash(hHash);
        if (pbHashObject)
            HeapFree(GetProcessHeap(), 0, pbHashObject);
        if (pbHash)
            HeapFree(GetProcessHeap(), 0, pbHash);
    } 
}
*/
