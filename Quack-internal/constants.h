#pragma once
#include "pch.h"

namespace constants {
    const LPCWSTR DLL_NAME { L"Quack-internal" };
    const std::string VERSION { "0.2.1" };
    constexpr unsigned IPC_PORT = 5175; // Local machine communications port
    constexpr unsigned NET_PORT = 7982; // Foreign network communications port
    static constexpr bool DBG = true;
}
