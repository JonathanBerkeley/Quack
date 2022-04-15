// pch.hpp: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// Win32 API
#define _WINSOCKAPI_
#define NOMINMAX
#include <Windows.h>
#include <iphlpapi.h>

// C++ Standard library
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#endif PCH_H
