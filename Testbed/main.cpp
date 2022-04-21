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
#include <algorithm>
#include <ranges>



std::wstring wstring_to_lower(const std::wstring& wide_str) {

    std::wstring output;

    for (auto& wc : wide_str)
        output += static_cast<wchar_t>(towlower(wc));

    return output;
}

using ProcessEntry = std::unordered_map<DWORD, std::wstring>;
std::optional<ProcessEntry> find_processes(const std::vector<std::wstring>& blacklist) {

    ProcessEntry entries_found{};

    PROCESSENTRY32 process_entry{};
    process_entry.dwSize = sizeof(PROCESSENTRY32);

    // ReSharper disable once CppLocalVariableMayBeConst
    HANDLE proc_snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (!Process32First(proc_snap, &process_entry))
        return std::nullopt;

    // Loop over processes and look for name
    while (Process32Next(proc_snap, &process_entry)) {

        std::wstring process_name{ process_entry.szExeFile };
        // Create lower-string representations
        process_name = wstring_to_lower(process_name);

        // Check blacklist for entry
        for (const auto& entry : blacklist)
            if (process_name.find(entry) != std::wstring::npos)
                // A process with a blacklisted name was found, so store it
                entries_found[process_entry.th32ProcessID] = process_entry.szExeFile;

    }

    // Cleanup handle
    CloseHandle(proc_snap);

    if (entries_found.empty())
        return std::nullopt;

    return entries_found;
}


/**
 * \brief Prints the name of process and it's process ID, or <Untitled> if the process name is unknown
 *
 * Adapted from https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-enumprocesses
 * \param processID ID of process to print name and ID of
 */
void PrintProcessNameAndID(DWORD processID) {

    struct Process {
        TCHAR name[MAX_PATH];
        HANDLE handle;
    } process{
        L"<Untitled>",
        OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,FALSE, processID)
    };

    // Get the process name
    if (process.handle != nullptr) {
        HMODULE h_mod;
        DWORD cb_needed;


        if (EnumProcessModules(process.handle, &h_mod, sizeof h_mod, &cb_needed))
            GetModuleBaseName(process.handle, h_mod, process.name, sizeof process.name / sizeof TCHAR);
    }

    // Print the process name and ID
    std::wcout << process.name << " PID: " << processID << '\n';

    // Cleanup
    if (process.handle)
        CloseHandle(process.handle);
}


void Testbed() {

    DWORD aProcesses[1024], cbNeeded;

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
        return;
    }

    // Calculate how many process identifiers were returned
    const DWORD process_count = cbNeeded / sizeof DWORD;

    // Print the name and process identifier for each process
    for (unsigned i = 0; i < process_count; i++) {
        if (aProcesses[i] != 0) {
            PrintProcessNameAndID(aProcesses[i]);
        }
    }

}


int main() {

    AllocConsole();
    freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
    freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);

    const std::vector blacklist{
        std::wstring{ L"cheatengine" },
        std::wstring{ L"xenos" },
        std::wstring{ L"extremeinjector" },
        std::wstring{ L"destroject" }
    };

    if (const auto processes = find_processes(blacklist)
        ;  processes) {

        for (const auto& [pid, name] : processes.value()) {
            std::wcout << pid << name << L'\n';
            auto h = OpenProcess(PROCESS_ALL_ACCESS, false, 32564);
            if (not TerminateProcess(h, 0)) {
                std::wcout << GetLastError() << L'\n';
            }


        }
    }

    // Testbed();

    Sleep(100'000);
    return 0;
}