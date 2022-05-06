#include "pch.hpp"

#include "config.hpp"
#include "constants.hpp"
#include "heartbeat.hpp"
#include "cpu_counter.hpp"
#include "hardware_id.hpp"
#include "utils.hpp"

namespace chrono = std::chrono;
namespace thread = std::this_thread;

using namespace std::chrono_literals;
using constants::DBG;


void HeartbeatWrapper(const Context& ctx) {
    // Override check so that testing can be done without server connection
    if (not Heartbeat(ctx))
        if constexpr (not DBG)
            ExitFailure(cfg::ExitCode::NoHeartbeat);
}


/**
 * \brief Main loop of anti-cheat module which sends heartbeats and decides which tasks to run
 */
void TaskDispatch(const Context& ctx) {

    HardwareID hwid;
    Log("HWID raw: " + hwid.GetRawHWID());
    Log("HWID hash: " + hwid.GetHash());

    const CpuCounter cpu_usage{};
    constexpr auto sleep_delay = 1s;

    // Main event loop of module
    for (auto seconds = 0s; ; ++seconds) {
        // Avoid exhausting CPU
        for (unsigned skip_count = 0; cpu_usage() > cfg::cpu_usage_threshold; ++skip_count) {

            // If CPU is still exhausted after several seconds, continue anyways
            if (skip_count > 10)
                break;

            HeartbeatWrapper(ctx);

            thread::sleep_for(sleep_delay);
        }

        // Task dispatch system to avoid running multiple expensive tasks at once
        switch (seconds.count()) {
        case 1: {
        } break;
        case 5: {
        } break;
        case 10: {
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

        HeartbeatWrapper(ctx);

        thread::sleep_for(sleep_delay);
    }

}
