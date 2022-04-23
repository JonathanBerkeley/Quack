// Win32 API
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#define NOMINMAX
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>

#include <iostream>
#include <cstdio>
#include <sstream>
#include <optional>
#include <unordered_map>

int main() {

    AllocConsole();
    freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
    freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);


    auto x = GetModuleHandle(nullptr);
    auto y = GetCurrentProcess();

    std::wcout << x;

    std::wstring ws;
    std::wcin >> ws;
    return 0;
}
