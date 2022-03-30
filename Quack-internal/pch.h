// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.hpp" // Windows

#include <iostream>
#include <vector>
#include <algorithm>
#include <ranges>
#include <string>
#include <utility>
#include <chrono>
#include <thread>
#include <optional>
#include <cstdio>
#include <cstdlib>

// Ignore specific warnings from 3rd party libraries
#pragma warning(push)
#pragma warning(disable : 26478 6031)

// 3rd party library files
#include "Lib/httplib.hpp"
#include "Lib/json.hpp"

#pragma warning(pop)

#include <tchar.h>
#include <Psapi.h>

#endif //PCH_H
