#pragma once
#include "pch.hpp"
#include "constants.hpp"


class Communication {
private:
    inline static httplib::Client cli{ "localhost", constants::IPC_PORT };
public:
    static bool SendData(const nlohmann::json& body);
};