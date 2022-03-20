#include "pch.h"

// This project
#include "constants.h"
#include "heartbeat.h"

// 3rd party library
#include "Lib/httplib.hpp"  // Networking
#include "Lib/json.hpp"     // JSON support

// Shorten namespace names
namespace http = httplib;
namespace json = nlohmann;
namespace thread = std::this_thread;
namespace chrono = std::chrono;

using constants::DBG;

bool Heartbeat(const Context& ctx) {

    const auto uptime{
        chrono::duration_cast<chrono::seconds>(chrono::system_clock::now() - ctx.start_point)
    };

    json::json body{};
    // todo: hash real data and send it
    body["heartbeat"] = {
        {"id", "uuid1273198439343492237401"},
        {"name", "legit_player"},
        {"uptime", uptime.count()},
        {"blob", {
            {"Game-Specific-Info", "Important data!"}
        }}
    };

    if constexpr (DBG) {
        SetConsoleTextAttribute(ctx.hConsole, 7);
        std::cout << "\nHeartbeat uptime " << uptime.count() << "...\n";
    }

    if (auto res = ctx.cli->Post("/", body.dump(), "application/json")) {
        if (res->status == 200) {
            if constexpr (DBG) {
                SetConsoleTextAttribute(ctx.hConsole, 2);
                std::cout << res->body << std::endl;
            }
            return true;
        }
    }
    else {
        if constexpr (DBG) {
            SetConsoleTextAttribute(ctx.hConsole, 12);
            const auto err = res.error();
            std::cout << "Error: " << http::to_string(err) << std::endl;
        }
        return false;
    }

    return false;
}