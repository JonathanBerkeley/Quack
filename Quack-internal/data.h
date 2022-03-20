#pragma once
#include "pch.h"

namespace data {
    // Used for looping, setting to false will deactivate continued module functionality
    extern bool running;
    
    // Information about the process that this .DLL is inside
    struct ProcessInfo {
        // Process specific
        HANDLE hProcess;
        DWORD pid;

        // Module specific
        HMODULE this_module;
    };
    
    /**
     * \brief Maintains a list of known cheat signatures with an associated name
     */
    class Signatures {
    private:
        using name = std::string;
        using pattern = std::vector<std::string>;
    public:
        std::vector<std::pair<name, pattern>> cheats{};
    };
}
