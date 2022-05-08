#include "pch.hpp"

// This project
#include "constants.hpp"
#include "server.hpp"
#include "context.hpp"

// 3rd party library
#include "httplib.hpp"              // Networking
#include "json.hpp"                 // JSON support
#include "task_dispatch.hpp"

// Shorten namespace names
namespace http = httplib;
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
    
    // Enter the task dispatch loop
    TaskDispatch(context);
}
