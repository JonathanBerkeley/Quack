// Win32 API
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#define NOMINMAX
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Windows.h>
#include <Psapi.h>

#include <iostream>
#include <cstdio>
#include <sstream>


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
    } process {
        L"<Untitled>",
        OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,FALSE, processID)
    };

    // Get the process name
    if (process.handle != nullptr) {
        HMODULE h_mod;
        DWORD cb_needed;

        if (EnumProcessModules(process.handle, &h_mod, sizeof h_mod, &cb_needed))
            GetModuleBaseName(process.handle, h_mod, process.name,sizeof process.name / sizeof TCHAR);
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

    Testbed();

    return 0;
}