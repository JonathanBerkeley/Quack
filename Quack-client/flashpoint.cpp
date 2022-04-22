#include "pch.hpp"

// This project
#include "constants.hpp"
#include "server.hpp"
#include "context.hpp"
#include "heartbeat.hpp"
#include "utility.hpp"
#include "identification.hpp"

// 3rd party library
#include "detection.hpp"
#include "Lib/httplib.hpp"              // Networking
#include "Lib/json.hpp"                 // JSON support

// Shorten namespace names
namespace http = httplib;
namespace thread = std::this_thread;
namespace chrono = std::chrono;

using namespace std::chrono_literals;
using constants::DBG;


int main() {
    // todo: implement risk factor
    unsigned risk_factor = 0;

    Context context;

    if constexpr (DBG) {
        // Console for debug
        AllocConsole();
        freopen_s(recast<FILE**>(stdin), "CONIN$", "r", stdin);
        freopen_s(recast<FILE**>(stdout), "CONOUT$", "w", stdout);
        freopen_s(recast<FILE**>(stderr), "CONOUT$", "w", stderr);
        const auto hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        context.hConsole = hConsole;
    }

    const std::vector blacklist{
        std::wstring{ L"cheatengine" },
        std::wstring{ L"xenos" },
        std::wstring{ L"injector" },
        std::wstring{ L"destroject" }
    };

    auto x = KillBlacklistedProcesses(blacklist);
    if (x) {
        // todo: send info about detections?
        // todo: risk factor
        // todo: network blacklist
        risk_factor += x * 10u;
    }

    if (const auto arp_hashes = GetArpMacHashes(); arp_hashes) {
        for (const auto& hash : arp_hashes.value()) {
            Log(hash);
            // todo: use this information
        }
    }

    http::Client cli{ "localhost", constants::NET_PORT };

    // Start inter-process communication server
    auto ipc_server{ std::thread{ Server, &cli }};

    context.start_point = chrono::system_clock::now();
    context.cli = &cli;

    // Heart of the application, main loop
    for (;;) {
        Heartbeat(context);

        // Simple anti-debugger check
        if constexpr (not DBG)
            if (IsDebuggerPresent())
                return -1;

        thread::sleep_for(3s);
    }
}
