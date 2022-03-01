#pragma once
#include "pch.h"

// Information about the process that this .DLL is inside
struct ProcessInfo {
    // Process specific
    HANDLE hProcess;
    DWORD pid;

    // Module specific
    HMODULE this_module;
};


[[nodiscard]] std::vector<HMODULE> EnumerateModules(ProcessInfo pi);
bool VerifyModule(LPCWSTR source_file);
std::uint8_t* PatternScan(HMODULE module, const char* signature);
