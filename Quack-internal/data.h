#pragma once
#include "pch.h"

namespace data {
    // Used for looping, setting to false will deactivate continued module functionality
    extern bool running;

    namespace proc {
        // Windows
        extern HMODULE self_module;
    }
}