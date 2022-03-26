#include "pch.h"

#include "utils.h"
#include "constants.h"
#include "network.h"

#include "Lib/httplib.hpp"
#include "Lib/json.hpp"

namespace http = httplib;
namespace json = nlohmann;

using namespace std::string_literals;


bool Communication::SendData(const json::json& body) {
    if (auto res = cli.Post("/", body.dump(), "application/json")) {
        if (res->status == 200) {
            Log(res->body);
            return true;
        }
        Log("Failed: " + std::to_string(res->status));
        Log({ "\nError:"s, to_string(res.error()) });
        return false;
    }

    return false;
}
