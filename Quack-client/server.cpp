#include "pch.hpp"

#include "server.hpp"
#include "constants.hpp"

// 3rd party
#include "httplib.hpp"
#include "json.hpp"

namespace http = httplib;
using namespace std::chrono_literals;


void Server(http::Client* cli) {
    http::Server server{};
    bool connected = true;

    // Exit process if internal communication has been lost
    auto process_monitor = std::async(std::launch::async, [&connected] {
        while (connected) {
            connected = false;
            std::this_thread::sleep_for(3s);
        }
        ExitProcess(0);
    });

    // Routes
    server.Post("/", [cli, &connected](const http::Request& req, http::Response& res) {

        if (req.body.find("detection") != std::string::npos) {

            auto response = cli->Post("/ac", req.body, "application/json");

            res.set_content("QAC: Cheat info received", "text/plain");
        }
        else {
            connected = true;
            res.set_content("QAC: Heartbeat received", "text/plain");
        }
    });

    server.Get("/signatures", [cli](const http::Request& req, http::Response& res) {

        if (auto raw_signatures = cli->Get("/data/signatures")) {
            res.set_content(raw_signatures->body, "application/json");
        }
    });

    server.listen("localhost", constants::IPC_PORT);
}
