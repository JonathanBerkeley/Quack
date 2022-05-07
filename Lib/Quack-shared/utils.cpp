// ReSharper disable CppClangTidyClangDiagnosticCoveredSwitchDefault
#include "pch.hpp"
#include "utils.hpp"
#include "config.hpp"


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
    case ExitCode::BlacklistedProgram:
        exit_message = L"Blacklisted program found";
        break;
    default:
        exit_message = L"Unknown";
        break;
    }

    Log(exit_message);
    ExitProcess(exit_code);
}


/**
 * \brief Convert a std::wstring to a std::string
 * \param wide_str Wide string to be converted
 * \return std::string representation of the std::wstring provided on success, std::nullopt on failure
 */
std::optional<std::string> wstring_to_string(const std::wstring& wide_str) {
    if (wide_str.empty())
        return {};

    const auto size = WideCharToMultiByte(
        CP_UTF8,
        0,
        &wide_str[0],
        static_cast<int>(wide_str.size()),
        nullptr,
        0,
        nullptr,
        nullptr
    );

    if (size <= 0)
        return {};

    std::string output(size, 0);
    WideCharToMultiByte(
        CP_UTF8,
        0,
        &wide_str[0],
        static_cast<int>(wide_str.size()),
        &output[0],
        size,
        nullptr,
        nullptr
    );

    return output;
}
