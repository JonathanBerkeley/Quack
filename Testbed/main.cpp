// Win32 API
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#define NOMINMAX
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Windows.h>
#include <WinSock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <cstdio>
#include <string>


void Testbed() {
    // Identification
    // todo: Develop
    MIB_IPNET_TABLE2* arp;

    const auto result = GetIpNetTable2(AF_UNSPEC, &arp);
    if (result == NO_ERROR || result == ERROR_NOT_FOUND) { // Success

        for (DWORD i = 0; i < arp->NumEntries; ++i) {
            
            printf("Physical Address[%d]:\t ", (int)i);
            if (arp->Table[i].PhysicalAddressLength == 0)
                printf("\n");

            for (unsigned long j = 0; j < arp->Table[i].PhysicalAddressLength; j++) {
                if (j == arp->Table[i].PhysicalAddressLength - 1)
                    std::cout << std::hex << (int)arp->Table[i].PhysicalAddress[j] << '\n';
                else
                    std::cout << std::hex << (int)arp->Table[i].PhysicalAddress[j] << '-';
            }


            auto currentbit = static_cast<int>(arp->Table[i].PhysicalAddress[0]);
            std::cout << currentbit << '\n';
            if (currentbit == 0 || currentbit == 255)
                continue;

            for (unsigned long j = 0; j < arp->Table[i].PhysicalAddressLength; j++) {
                
                if (j == arp->Table[i].PhysicalAddressLength - 1) {
                    std::cout << std::dec << (int)arp->Table[i].PhysicalAddress[j] << '\n';
                }
                else {
                    std::cout << std::dec << (int)arp->Table[i].PhysicalAddress[j] << '-';
                }
            }

            //std::cout << arp->Table[i].PhysicalAddress << '\n';
        }

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