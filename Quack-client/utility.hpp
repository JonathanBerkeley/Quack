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

/**
 * \brief Constructs an std::optional of type T if T is not .empty()
 *
 * Only use in a return statement, argument is std::move'd.
 *
 * Thanks to fux for help with this.
 * \tparam T Any type with .empty() method
 * \param t Container to make std::optional from
 * \return std::optional<> with value or std::nullopt
 */
template <typename T>
std::optional<std::remove_reference_t<T>> NotEmpty(T&& t) {
    if (t.empty())
        return {};
    return std::move(t);  // NOLINT(bugprone-move-forwarding-reference) Only used for return statements
}

std::wstring wstring_to_lower(const std::wstring& wide_str);
