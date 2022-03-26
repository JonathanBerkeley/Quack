#pragma once

#include <future>
#include <string>
#include <initializer_list>

#include "constants.h"

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
 * \brief Calls a function with one argument asynchronously
 *
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
