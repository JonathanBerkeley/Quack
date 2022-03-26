#pragma once

#include "constants.h"

#include "Lib/httplib.hpp"
#include "Lib/json.hpp"

class Communication {
private:
    inline static httplib::Client cli{ "localhost", constants::IPC_PORT };
public:
    static bool SendData(const nlohmann::json& body);
};
