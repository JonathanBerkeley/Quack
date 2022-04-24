#include "pch.hpp"
#include "flashpoint.hpp"
#include "constants.hpp"
#include "data.hpp"
#include "dns_walk.hpp"
#include "config.hpp"
#include "cpu_counter.hpp"
#include "network.hpp"
#include "memory_scanner.hpp"
#include "task_dispatch.hpp"
#include "utils.hpp"

namespace json = nlohmann;

namespace thread = std::this_thread;

using namespace std::chrono_literals;
using namespace cfg;
using namespace data;
using constants::DBG;


[[noreturn]]
void TaskDispatch() {

    const ProcessInfo process_info{
        GetCurrentProcess(),
        GetCurrentProcessId(),
        GetModuleHandle(nullptr)
    };

    const CpuCounter cpu_usage{};

    const json::json heartbeat{
        {"Heartbeat", true}
    };


    const std::vector<DWORD> mask{ PAGE_EXECUTE, PAGE_EXECUTE_READWRITE, PAGE_EXECUTE_READ, PAGE_EXECUTE_WRITECOPY };
    for (;;) {
        MEMORY_BASIC_INFORMATION mbi{};
        LPVOID offset = nullptr;
        while (VirtualQueryEx(GetCurrentProcess(), offset, &mbi, sizeof mbi)) {
            offset = reinterpret_cast<LPVOID>(reinterpret_cast<DWORD_PTR>(mbi.BaseAddress) + mbi.RegionSize);

            if (std::ranges::find(mask, mbi.AllocationProtect) != mask.end()) {
                // todo: scan

                MemoryRegion memory_region{
                    .size = mbi.RegionSize,
                    .start_address = static_cast<std::uint8_t*>(mbi.BaseAddress)
                };

                ModuleScan(process_info, memory_region);

                //std::wcout << L"\n0x" << std::hex << mbi.BaseAddress << L"\n";
                //std::wcout << mbi.AllocationProtect << L"\n";
            }
        }
        Sleep(15'000);
    }


    // todo: Sleep instead of exit when cpu usage is too high
    while (running) {

        // Simple anti-debugger check
        if constexpr (not DBG)
            if (IsDebuggerPresent())
                ExitProcess(-1);

        // Avoid exhausting CPU
        for (unsigned skip_count = 0; cpu_usage() > cpu_usage_threshold; ++skip_count) {

            // If CPU is still exhausted, continue anyways
            if (skip_count > 10)
                break;

            Communication::SendData(heartbeat);
            thread::sleep_for(1s);
        }

        Communication::SendData(heartbeat);

        if constexpr (ModuleSigScanning)
            // Scan the modules in memory of target process
            ModuleScan(process_info, UnsignedModulesOnly);

        if constexpr (DNSScanning) {

            if (const auto entries = CheckForBlacklistedDNSEntries()) {
                Log("Blacklisted domain(s) found: ");

                for (const auto& [name, type] : entries.value()) {
                    // todo: move from here and network
                    std::wstring printable{ name + L" " + std::to_wstring(type) };
                    Log(printable);
                }
            }
        }

        thread::sleep_for(10s);
    }
}

