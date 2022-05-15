#pragma once
#include "pch.hpp"


/**
 * \brief Global constants
 */
namespace constants {
    const LPCWSTR W_DLL_NAME { L"Quack-internal" };
    const LPCSTR DLL_NAME{ "Quack-internal" };

    const std::string VERSION { "0.6.5" };
    constexpr unsigned IPC_PORT = 5175; // Local machine communications port
    constexpr unsigned NET_PORT = 7982; // Foreign network communications port
    static constexpr bool DBG = true;
}
