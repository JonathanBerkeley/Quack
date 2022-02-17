#define _WINSOCKAPI_
#include <Windows.h>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include "Lib/httplib.hpp"
#include "Lib/json.hpp"

// Make these library namespace names a bit nicer
namespace http = httplib;
namespace json = nlohmann;
namespace thread = std::this_thread;

using namespace std::chrono_literals;

int main() {
    // Console for debug
    AllocConsole();
    freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
    freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);

    const auto hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    http::Client cli{ "http://localhost:7982" };

    // Heart of the application, main loop
    for (unsigned i = 1u; ; ++i) {
        json::json body{};
        body["heartbeat"] = {
            {"id", "uuid1273198439343492237401"},
            {"name", "legit_player"},
            {"uptime", i},
            {"blob", {
                {"Game-Specific-Info", "Important data!"}
            }}
        };
        SetConsoleTextAttribute(hConsole, 7);
        std::cout << "\nSending heartbeat number " << i << "...\n";

        if (auto res = cli.Post("/", body.dump(), "application/json")) {
            if (res->status == 200) {
                SetConsoleTextAttribute(hConsole, 2);
                std::cout << res->body << std::endl;
            }
        }
        else {
            SetConsoleTextAttribute(hConsole, 12);
            const auto err = res.error();
            std::cout << "Error: "  << http::to_string(err) << std::endl;
        }
        thread::sleep_for(3s);
    }

    // Halt app to print error text
    std::string dummy;
    std::cin >> dummy;
    return 0;
}
