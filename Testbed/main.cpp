// Win32 API
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#define NOMINMAX
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Windows.h>
#include <Psapi.h>
#include <sysinfoapi.h>
#include <TlHelp32.h>
#include <d3d9.h>

#include "smbios.hpp"

#include <iostream>
#include <filesystem>
#include <cstdio>
#include <sstream>
#include <optional>
#include <unordered_map>


std::optional<DWORD> DiskID() {
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


std::optional<std::string> GpuID() {
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


char16_t GetCpuID() {
    // EAX, EBX, ECX, EDX
    int cpu_id[4] = { 0, 0, 0, 0 };
    __cpuid(cpu_id, 0);

    char16_t id_output = 0;
    const auto* ptr = reinterpret_cast<char16_t*>(cpu_id);

    for (char32_t i = 0; i < 8; ++i)
        id_output += ptr[i];

    return id_output;
}

int main() {
    AllocConsole();
    freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
    freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);


    auto current_path = std::filesystem::current_path();

    current_path += "/Quack-ac.exe";
    const auto application_name = current_path.generic_string();
    std::cout << application_name.c_str();

    std::wstring ws;
    std::wcin >> ws;
    return 0;
}
