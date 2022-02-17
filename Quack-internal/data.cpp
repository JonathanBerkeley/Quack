#include "pch.h"
#include "data.h"

namespace data {
    bool running = true;

    namespace proc {
        // Windows
        HMODULE self_module = nullptr;
    }
}