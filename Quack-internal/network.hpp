#pragma once
#include "pch.hpp"
#include "constants.hpp"


/**
 * \brief Wrapper around networking functionality
 */
class Communication {
private:
    inline static httplib::Client cli{ "localhost", constants::IPC_PORT };
public:
    static bool SendData(const nlohmann::json& body);
};

bool Heartbeat(const nlohmann::json& heartbeat_data, const int miss_tolerance);
