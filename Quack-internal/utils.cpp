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
