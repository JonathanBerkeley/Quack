#include "pch.h"

#include "server.h"
#include "constants.h"

#include "Lib/httplib.hpp"
#include "Lib/json.hpp"

namespace http = httplib;
namespace json = nlohmann;


void Server() {
    http::Server server{};
    server.Post("/", [](const http::Request& req, http::Response& res) {
        res.set_content("QAC: Received", "text/plain");
        });
    server.listen("localhost", constants::IPC_PORT);
}
