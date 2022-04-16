// Win32 API
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#define NOMINMAX
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Windows.h>
#include <WinSock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#include "sha256.h"

#include <iostream>
#include <cstdio>
#include <ranges>
#include <algorithm>
#include <string>
#include <sstream>
#include <vector>


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

        auto row = arp->Table[i];

        SHA256 sha256;
        std::stringstream ss{};

        for (unsigned long j = 0; j < row.PhysicalAddressLength; j++) {
            if (j == row.PhysicalAddressLength - 1)
                ss << std::hex << static_cast<int>(row.PhysicalAddress[j]);
            else
                ss << std::hex << static_cast<int>(row.PhysicalAddress[j]) << '-';
        }
        std::cout << ss.str() << '\n';
        std::cout << sha256(ss.str()) << '\n';
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