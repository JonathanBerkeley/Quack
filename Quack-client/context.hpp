#pragma once
#include "pch.hpp"
#include "httplib.hpp"


/**
 * \brief Application context that can be passed around to decouple code dependencies
 */
struct Context {
    httplib::Client* cli{};
    HANDLE hConsole{};
    std::chrono::time_point<std::chrono::system_clock> start_point{};
};
