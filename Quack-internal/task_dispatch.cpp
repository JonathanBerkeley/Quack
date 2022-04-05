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
        // todo: Detection logic
        // todo: Signature scanning
        // todo: Internal heartbeat

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

