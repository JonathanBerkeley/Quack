#include "pch.hpp"
#include "detection.hpp"
#include "config.hpp"
#include "utility.hpp"


/**
 * \brief Check running processes' image name for those contained in blacklist
 * \param blacklist List of names to search process names for
 * \return Map of results with PID and name for each process if found, std::nullopt otherwise
 */
std::optional<ProcessEntry> FindProcesses(const std::vector<std::wstring>& blacklist) {

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

    return IfNotEmpty(entries_found);
}


/**
 * \brief Scans running processes image names against blacklist, kills any occurrences found
 * \param blacklist Names of processes to search for substring of in the image name of active processes
 * \return Count of processes killed
 */
unsigned KillBlacklistedProcesses(const std::vector<std::wstring>& blacklist) {
    if (const auto processes = FindProcesses(blacklist)
        ; processes) {

        unsigned count = 0;

        // For each blacklisted process found
        for (const auto& [pid, name] : processes.value()) {

            Log(std::wstring{ L"Killed process: " + name + L" : " + std::to_wstring(pid) });

            const auto blacklisted_process = OpenProcess(PROCESS_TERMINATE, false, pid);
            if (not TerminateProcess(blacklisted_process, 0)) {
                Log({ GetLastError() });
                ExitProcess(cfg::ExitCode::BlacklistedProgram);
            }

            ++count;
        }
        return count;
    }

    return 0;
}
