#include "pch.h"

#include "memory_scanner.h"

std::vector<HMODULE> EnumerateModules(const ProcessInfo pi) {
    HMODULE modules[0x400];
    DWORD cbNeeded;

    std::vector<HMODULE> dlls;

    if (EnumProcessModules(pi.hProcess, modules, sizeof(modules), &cbNeeded)) {
        for (auto i = 0u; i < cbNeeded / sizeof(HMODULE); ++i) {
            dlls.push_back(modules[i]);
        }

    }

    return dlls;
}
