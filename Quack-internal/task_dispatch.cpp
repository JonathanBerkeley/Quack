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

    // todo: Sleep instead of exit when cpu usage is too high
    while (running) {

        // Simple anti-debugger check
        if constexpr (not DBG)
            if (IsDebuggerPresent())
                ExitProcess(-1);

        // Avoid exhausting CPU
        for (unsigned skip_count = 0; cpu_usage() > cfg::cpu_usage_threshold; ++skip_count) {

            // If CPU is still exhausted, continue anyways
            if (skip_count > 10)
                break;

            Communication::SendData(heartbeat);
            thread::sleep_for(1s);
        }

        Communication::SendData(heartbeat);

        if constexpr (cfg::Scan::DNS)
            // Scan the modules in memory of target process
            ModuleScan(process_info, cfg::Scan::UnsignedModulesOnly);

        if constexpr (cfg::Scan::ExecutableMemory)
            // Scan executable memory of target process
            FullScan(process_info);

        if constexpr (cfg::Scan::DNS) {

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

