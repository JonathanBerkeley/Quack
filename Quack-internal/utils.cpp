#include "pch.h"

#include "utils.h"
#include "constants.h"


void Log(const std::string& str) {
    if constexpr (constants::DBG)
        std::cout << str << '\n';
}

void Log(const std::wstring& w_str) {
    if constexpr (constants::DBG)
        std::wcout << w_str << '\n';
}
