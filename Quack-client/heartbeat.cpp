#include "pch.hpp"

// This project
#include "constants.hpp"
#include "heartbeat.hpp"
#include "task_dispatch.hpp"

// 3rd party library
#include "httplib.hpp"  // Networking
#include "json.hpp"     // JSON support

// Shorten namespace names
namespace http = httplib;
namespace json = nlohmann;
namespace chrono = std::chrono;

using constants::DBG;


bool Heartbeat(const Context& ctx, const HeartbeatInfo& heartbeat_info) {

    const auto uptime{
        chrono::duration_cast<chrono::seconds>(chrono::system_clock::now() - ctx.start_point)
    };

    json::json body{};
    body["heartbeat"] = {
        {"uuid", heartbeat_info.hwid},
        {"name", heartbeat_info.username},
        {"uptime", uptime.count()},
        {"blob", {
            {"Game specific data", "[Data]"}
        }}
    };

    if constexpr (DBG) {
        SetConsoleTextAttribute(ctx.hConsole, 7);
        std::cout << "\nHeartbeat uptime " << uptime.count() << "...\n";
    }

    if (auto res = ctx.cli->Post("/", body.dump(), "application/json")) {
        // Success
        if (res->status == 200) {
            if constexpr (DBG) {
                SetConsoleTextAttribute(ctx.hConsole, 2);
                std::cout << res->body << std::endl;
            }
            return true;
        }
    }
    // Failure
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