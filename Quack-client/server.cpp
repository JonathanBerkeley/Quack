#include "pch.hpp"

#include "server.hpp"
#include "constants.hpp"

#include "httplib.hpp"
#include "json.hpp"

namespace http = httplib;
namespace json = nlohmann;


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

    server.Post("/handshake", [cli](const http::Request& req, http::Response& res) {


        res.set_content("", "text/plain");
    });

    server.listen("localhost", constants::IPC_PORT);
}
