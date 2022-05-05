#include "pch.hpp"
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
namespace chrono = std::chrono;
namespace thread = std::this_thread;

using namespace std::chrono_literals;
using namespace data;
using constants::DBG;


// todo: send results to server
void DnsScan() {
    if (const auto entries = CheckForBlacklistedDNSEntries()) {
        Log("Blacklisted domain(s) found: ");

        for (const auto& [name, type] : entries.value()) {
            // todo: move from here and network
            std::wstring printable{ name + L" " + std::to_wstring(type) };
            Log(printable);
        }
    }
}


/**
 * \brief Main loop of anti-cheat module which sends heartbeats and decides which tasks to run
 */
void TaskDispatch() {

    const json::json heartbeat_data{
        {"Heartbeat", true}
    };

    const ProcessInfo process_info{
        GetCurrentProcess(),
        GetCurrentProcessId(),
        GetModuleHandle(nullptr)
    };

    const CpuCounter cpu_usage{};
    constexpr auto sleep_delay = 1s;

    // Main event loop of module
    for (auto seconds = 0s; running; ++seconds) {

        // Avoid exhausting CPU
        for (unsigned skip_count = 0; cpu_usage() > cfg::cpu_usage_threshold; ++skip_count) {

            // If CPU is still exhausted after several seconds, continue anyways
            if (skip_count > 10)
                break;

            if (not Heartbeat(heartbeat_data, 5))
                ExitFailure(cfg::ExitCode::NoHeartbeat);

            thread::sleep_for(sleep_delay);
        }

        // Task dispatch system to avoid running multiple expensive tasks at once
        switch (seconds.count()) {
        case 1: {
            if constexpr (cfg::Module)
                ModuleScan(process_info, cfg::UnsignedModulesOnly);
        } break;
        case 5: {
            if constexpr (cfg::DNS)
                DnsScan();
        } break;
        case 10: {
            if constexpr (cfg::ExecutableMemory)
                FullScan(process_info);
        } break;
        default: {
            // Simple anti-debugger check
            if constexpr (not DBG)
                if (IsDebuggerPresent())
                    ExitFailure(cfg::ExitCode::DebuggerPresent);
        } break;
        }

        if (seconds > 15s)
            seconds = 0s;

        if (not Heartbeat(heartbeat_data, 5))
            ExitFailure(cfg::ExitCode::NoHeartbeat);

        thread::sleep_for(sleep_delay);
    }

    ExitProcess(cfg::ExitCode::Success);
}
