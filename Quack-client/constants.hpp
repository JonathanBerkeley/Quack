#pragma once
#include <string>

namespace constants {
    const std::string VERSION{ "0.2.3" };
    const std::string NAME{ "Quack" };
    constexpr unsigned IPC_PORT = 5175; // Local machine communications port
    constexpr unsigned NET_PORT = 7982; // Foreign network communications port
    static constexpr bool DBG = true;
}
