#include "pch.hpp"

#include "utils.hpp"
#include "config.hpp"
#include "constants.hpp"


/**
 * \brief Logs to standard output conditionally if debug mode is enabled
 *
 * Undefined behaviour if used in conjunction with Log(const std::wstring& w_str)
 * \param str String to be printed
 */
void Log(const std::string& str) {
    if constexpr (constants::DBG)
        std::cout << str << '\n';
}

/**
 * \brief Logs to standard output conditionally if debug mode is enabled
 *
 * Undefined behaviour if used in conjunction with Log(const std::string& str)
 * \param w_str Wide character string to be printed
 */
void Log(const std::wstring& w_str) {
    if constexpr (constants::DBG)
        std::wcout << w_str << '\n';
}

/**
 * \brief Cache queries to LoadLibrary, to return a library reference if it is already loaded.
 *
 * Narrow contract, ensure dll_name exists before calling.
 * \param dll_name Wide character name of library to load or return
 * \return HMODULE for requested library
 */
HMODULE CachedLoadLibrary(const LPCWSTR& dll_name) {
    static std::unordered_map<LPCWSTR, HMODULE> cache{};

    if (cache.contains(dll_name))
        return cache[dll_name];

    cache[dll_name] = LoadLibrary(dll_name);
    return cache[dll_name];
}


/**
 * \brief Wrapper around ExitProcess
 *
 * This should be used for any process failure exit, so that future exit handling refactoring is easier
 * \param exit_code Reason for exiting
 */
void ExitFailure(const UINT exit_code) {
    using cfg::ExitCode;

    std::wstring exit_message;

    switch (static_cast<cfg::ExitCode>(exit_code)) {
    case ExitCode::Success:
        ExitProcess(0);
    case ExitCode::DebuggerPresent:
        exit_message = L"Debugger detected";
        break;
    case ExitCode::NoHeartbeat:
        exit_message = L"Anti-cheat lost connection";
        break;
    }

    Log(exit_message);
    ExitProcess(exit_code);
}
