#include "pch.hpp"

#include "utils.hpp"
#include "network.hpp"

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


/**
 * \brief Sends heartbeats to main anti-cheat module.
 *
 * Will cause process to exit if there are miss_tolerance amount of timeouts while waiting for response.
 * \param heartbeat_data Heartbeat json data to be sent to main anti-cheat module
 * \param miss_tolerance The number of timeouts to allow before exiting
 * \return False if there were more timeouts than miss_tolerance, true otherwise
 */
bool Heartbeat(const json::json& heartbeat_data, const int miss_tolerance) {
    static int consecutive_misses = 0;

    if (not Communication::SendData(heartbeat_data)) {
        ++consecutive_misses;
        Log(L"Missed " + std::to_wstring(consecutive_misses) + L"/" + std::to_wstring(miss_tolerance) + L" heartbeats");
    }
    else {
        consecutive_misses = 0;
    }

    return consecutive_misses < miss_tolerance;
}
