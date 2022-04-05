#include "pch.hpp"

// This project
#include "constants.hpp"
#include "server.hpp"
#include "context.hpp"
#include "heartbeat.hpp"

// 3rd party library
#include "Lib/httplib.hpp"  // Networking
#include "Lib/json.hpp"     // JSON support

// Shorten namespace names
namespace http = httplib;
namespace json = nlohmann;
namespace thread = std::this_thread;
namespace chrono = std::chrono;

using namespace std::chrono_literals;
using constants::DBG;

int main() {
    Context context;

    if constexpr (DBG) {
        // Console for debug
        AllocConsole();
        freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);
        freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
        freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);
        const auto hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        context.hConsole = hConsole;
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
