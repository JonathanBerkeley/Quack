#include "pch.h"

#include "server.h"
#include "constants.h"

#include "Lib/httplib.hpp"
#include "Lib/json.hpp"

namespace http = httplib;
namespace json = nlohmann;


void Server(http::Client* cli) {
    http::Server server{};
    server.Post("/", [cli](const http::Request& req, http::Response& res) {

        // todo: enum flags instead of literals
        if (req.body.find("Highlight") != std::string::npos) {

            auto response = cli->Post("/ac", req.body, "application/json");
            if (response->status != 200) {
                // todo
                ExitProcess(EXIT_FAILURE);
            }

            res.set_content("QAC: Cheat info received", "text/plain");
        }
        else {
            res.set_content("QAC: Heartbeat received", "text/plain");
        }
    });

    server.listen("localhost", constants::IPC_PORT);
}
