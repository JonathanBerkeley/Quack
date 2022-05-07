#include "pch.hpp"

#include "utility.hpp"
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
 * \brief Returns a lowercase version of wide string
 * \param wide_str Wide string to be converted
 * \return New wide string in lowercase
 */
std::wstring wstring_to_lower(const std::wstring& wide_str) {

    std::wstring output;

    for (auto& wc : wide_str)
        output += static_cast<wchar_t>(towlower(wc));

    return output;
}
