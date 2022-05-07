#include "pch.hpp"
#include "config.hpp"
#include "constants.hpp"
#include "heartbeat.hpp"
#include "cpu_counter.hpp"
#include "detection.hpp"
#include "task_dispatch.hpp"
#include "hardware_id.hpp"
#include "identification.hpp"
#include "utils.hpp"

namespace thread = std::this_thread;

using namespace std::chrono_literals;
using constants::DBG;


/**
 * \brief Wrapper around heartbeat call
 * \param ctx Application context
 * \param heartbeat_info Heartbeat information to be sent to the remote server
 */
void HeartbeatWrapper(const Context& ctx, const HeartbeatInfo& heartbeat_info) {
    // Override check so that testing can be done without server connection
    if (not Heartbeat(ctx, heartbeat_info))
        if constexpr (not DBG)
            ExitFailure(cfg::ExitCode::NoHeartbeat);
}


/**
 * \brief Main loop of anti-cheat module which sends heartbeats and decides which tasks to run
 * \param ctx Application context
 */
void TaskDispatch(const Context& ctx) {

    // todo: network
    const std::vector blacklist{
        std::wstring{ L"cheatengine" },
        std::wstring{ L"xenos" },
        std::wstring{ L"injector" },
        std::wstring{ L"destroject" }
    };

    // Get hardware id
    HardwareID hwid;
    Log("\nHWID raw: " + hwid.GetRawHWID());
    Log("HWID hash: " + hwid.GetHash());

    HeartbeatInfo heartbeat_info{
        .hwid = hwid.GetHash(),
        .username = "Placeholder"
    };

    const CpuCounter cpu_usage{};
    constexpr auto sleep_delay = 1s;

    // Main event loop of module
    for (auto seconds = 0s; ; ++seconds) {

        // Avoid exhausting CPU
        if constexpr (cfg::CpuThrottle) {
            for (unsigned skip_count = 0; cpu_usage() > cfg::cpu_usage_threshold; ++skip_count) {

                // If CPU is still exhausted after several seconds, continue anyways
                if (skip_count > 10)
                    break;

                HeartbeatWrapper(ctx, heartbeat_info);

                thread::sleep_for(sleep_delay);
            }
        }

        // ReSharper disable once CppTooWideScope
        unsigned risk_factor = 0u;

        // Task dispatch system to avoid running multiple expensive tasks at once
        switch (seconds.count()) {
        case 1: {

            if (auto arp_hashes = GetArpMacHashes(); arp_hashes) {

                if constexpr (DBG)
                    for (const auto& hash : arp_hashes.value())
                        Log(hash);

                heartbeat_info.arp_hashes = arp_hashes.value();
            }
        } break;
        case 5: {
            if (const auto processes_killed = KillBlacklistedProcesses(blacklist)) {
                risk_factor += processes_killed * 10u;

                heartbeat_info.risk_factor = risk_factor;
            }
        } break;
        case 10: {
        } break;
        default: {
            // Simple anti-debugger check
            if constexpr (not DBG) {
                if (IsDebuggerPresent()) {
                    risk_factor += 50u;
                    ExitFailure(cfg::ExitCode::DebuggerPresent);
                }
            }
        } break;
        }


        if (seconds > 15s)
            seconds = 0s;

        HeartbeatWrapper(ctx, heartbeat_info);
        thread::sleep_for(sleep_delay);
    }
}
