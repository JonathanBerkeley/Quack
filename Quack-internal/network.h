#pragma once

#include "constants.h"

#include "Lib/httplib.hpp"
#include "Lib/json.hpp"

class Communication {
private:
    httplib::Client cli{ "localhost", constants::IPC_PORT };
public:
    void SendData(const nlohmann::json& body);
};
