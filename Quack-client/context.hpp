#pragma once

#include <Windows.h>
#include "httplib.hpp"

// Context that can be passed around to decouple code dependencies
struct Context {
    httplib::Client* cli{};
    HANDLE hConsole{};
    std::chrono::time_point<std::chrono::system_clock> start_point{};
};
