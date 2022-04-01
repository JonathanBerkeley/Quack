#include "pch.hpp"

#include "utils.hpp"
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
