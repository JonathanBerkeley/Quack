#include "pch.hpp"

#include "server.hpp"
#include "constants.hpp"

#include "Lib/httplib.hpp"
#include "Lib/json.hpp"

namespace http = httplib;
namespace json = nlohmann;


void Server(http::Client* cli) {
    http::Server server{};

    // Routes
    server.Post("/", [cli](const http::Request& req, http::Response& res) {

        // todo: enum flags instead of literals
        // todo: proof of identity
        if (req.body.find("Highlight") != std::string::npos) {

            auto response = cli->Post("/ac", req.body, "application/json");
            if (response->status != 200) {
                
                // todo
                // ExitProcess(EXIT_FAILURE);
            }

            res.set_content("QAC: Cheat info received", "text/plain");
        }
        else {
            res.set_content("QAC: Heartbeat received", "text/plain");
        }
    });


    server.listen("localhost", constants::IPC_PORT);
}
