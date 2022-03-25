#include "pch.h"

#include "utils.h"
#include "constants.h"
#include "network.h"

#include "Lib/httplib.hpp"
#include "Lib/json.hpp"

namespace http = httplib;
namespace json = nlohmann;


void Communication::SendData(const json::json& body) {

    if (auto res = cli.Post("/", body.dump(), "application/json")) {
        if (res->status == 200) {
            Log(res->body);
        }
        else {
            Log("Failed: " + res->status);
            // std::cout << "\nError:" << res.error();
        }
    }
}
