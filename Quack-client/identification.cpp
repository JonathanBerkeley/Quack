#include "pch.hpp"

#include "utility.hpp"
#include "identification.hpp"

#include "Lib/hash-library/sha256.hpp"
#pragma comment(lib, "iphlpapi.lib")


using Hashes = std::vector<std::string>;
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

        hashes.emplace_back(
            sha256(ss.str())
        );

        Log(sha256(ss.str()));
    }

    if (hashes.empty())
        return std::nullopt;

    return hashes;
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
