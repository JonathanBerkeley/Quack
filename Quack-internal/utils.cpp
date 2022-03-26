#include "pch.h"

#include "utils.h"
#include "constants.h"


// Undefined behaviour if used in conjunction with Log(const std::wstring& w_str)
void Log(const std::string& str) {
    if constexpr (constants::DBG)
        std::cout << str << '\n';
}

// Undefined behaviour if used in conjunction with Log(const std::string& str)
void Log(const std::wstring& w_str) {
    if constexpr (constants::DBG)
        std::wcout << w_str << '\n';
}
