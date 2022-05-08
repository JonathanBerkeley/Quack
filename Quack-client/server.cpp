#include "pch.hpp"

#include "server.hpp"
#include "constants.hpp"

// 3rd party
#include "httplib.hpp"
#include "json.hpp"

namespace http = httplib;


void Server(http::Client* cli) {
    http::Server server{};

    // Routes
    server.Post("/", [cli](const http::Request& req, http::Response& res) {

        if (req.body.find("detection") != std::string::npos) {

            auto response = cli->Post("/ac", req.body, "application/json");

            res.set_content("QAC: Cheat info received", "text/plain");
        }
        else {
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
