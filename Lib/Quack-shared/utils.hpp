#pragma once

#include <future>
#include <string>
#include <initializer_list>

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
 * \brief Calls a function with one argument asynchronously
 *
 * Relies on implementation defined behaviour
 * Adapted from: https://stackoverflow.com/a/56834117
 * \tparam F Function object type
 * \tparam T Function argument type
 * \param function Function object
 * \param function_arg Function argument
 */
template <typename F, typename T>
void CallAsync(F&& function, T&& function_arg) {
    auto future_ptr = std::make_shared<std::future<void>>();
    *future_ptr = std::async(std::launch::async, [future_ptr, function, function_arg]() {
        function(function_arg);
    });
}


/**
 * \brief Constructs an std::optional of type T if T is not .empty()
 *
 * Only use in a return statement, as argument is std::move'd.
 *
 * Thanks to "fux" on discord for help with this.
 * \tparam T Any type with .empty() method
 * \param t Container to make std::optional from
 * \return std::optional<> with value or std::nullopt
 */
template <typename T>
std::optional<std::remove_reference_t<T>> IfNotEmpty(T&& t) {
    if (t.empty())
        return {};
    return std::move(t);  // NOLINT(bugprone-move-forwarding-reference)
}


HMODULE CachedLoadLibrary(const LPCWSTR& dll_name);

[[noreturn]]
void ExitFailure(const UINT exit_code);

std::optional<std::string> wstring_to_string(const std::wstring& wide_str);
std::wstring wstring_to_lower(const std::wstring& wide_str);
