#include "pch.hpp"

// This project
#include "constants.hpp"
#include "server.hpp"
#include "context.hpp"
#include "heartbeat.hpp"
#include "utility.hpp"

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

#pragma comment(lib, "Iphlpapi.lib")

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

    // Identification
    // todo: Develop
    MIB_IPNETTABLE arp{};
    unsigned long arp_sz = 1024;
    auto result = GetIpNetTable(&arp, &arp_sz, TRUE);

    if (result == ERROR_INSUFFICIENT_BUFFER)
        result = GetIpNetTable(&arp, &arp_sz, TRUE);

    if (result == NO_ERROR) {

        for (DWORD i = 0; i < arp.dwNumEntries; ++i) {
            std::cout << "Arp: " << i << " " << arp.table[i].dwIndex << " : " << arp.table[i].dwAddr << '\n';
        }
    }
    else {
        Log({ "Error: " + std::to_string(result) });
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
