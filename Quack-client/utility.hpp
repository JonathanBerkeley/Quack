#pragma once

#include "constants.hpp"

void Log(const std::string& str);
void Log(const std::wstring& w_str);

/**
 * \brief Log various homogeneously typed data to standard output (through std::cout)
 * \tparam T Initializer_list of this type
 * \param data Data to be printed
 */
template <typename T>
void Log(std::initializer_list<T> data) {
    if constexpr (constants::DBG)
        for (auto& segment : data)
            std::cout << segment << '\n';
}

/**
 * \brief Wrapper around reinterpret_cast to reduce code verbosity
 * \tparam T Resulting type after casting
 * \tparam U Type of the subject of the cast
 * \param u Subject of the cast
 * \return Result of reinterpret_cast
 */
template <typename T, typename U>
T recast(U&& u) {
    return reinterpret_cast<T>(std::forward<U>(u));
}

std::wstring wstring_to_lower(const std::wstring& wide_str);
