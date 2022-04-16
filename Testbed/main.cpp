// Win32 API
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#define NOMINMAX
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Windows.h>
#include <WinSock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#include <bcrypt.h>
#include <wincrypt.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "Crypt32.lib")

#include <iostream>
#include <cstdio>
#include <ranges>
#include <algorithm>
#include <string>
#include <vector>


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


void Testbed() {
    
    // Identification
    // todo: Develop
    MIB_IPNET_TABLE2* arp;

    const auto result = GetIpNetTable2(AF_UNSPEC, &arp);
    if (result != NO_ERROR && result != ERROR_NOT_FOUND) return;
    
    const std::vector blacklist{ 0x0, 0xff, 0x33, 0x01 };

    for (DWORD i = 0; i < arp->NumEntries; ++i) {

        const auto current_bit = static_cast<int>(arp->Table[i].PhysicalAddress[0]);
        if (std::ranges::find(blacklist, current_bit) != blacklist.end())
            continue;

        for (unsigned long j = 0; j < arp->Table[i].PhysicalAddressLength; j++) {
            if (j == arp->Table[i].PhysicalAddressLength - 1)
                std::cout << std::hex << (int)arp->Table[i].PhysicalAddress[j] << '\n';
            else
                std::cout << std::hex << (int)arp->Table[i].PhysicalAddress[j] << '-';
        }
        Hash(arp->Table[i].PhysicalAddress);
    }
    
}

int main() {

    AllocConsole();
    freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
    freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);

    Testbed();

    return 0;
}