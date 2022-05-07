#pragma once
#include "pch.hpp"


/**
 * \brief Data structures needed for various parts of the application
 */
namespace data {
    // Used for looping, setting to false will deactivate continued module functionality
    inline bool running = true;

    /**
     * \brief Information about the process that this .DLL is inside
     */
    struct ProcessInfo {
        // Process specific
        HANDLE handle;
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
