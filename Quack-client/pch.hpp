// pch.hpp: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// Win32 API
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#define NOMINMAX

#include <Windows.h>
#include <WinSock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#include <Psapi.h>
#include <TlHelp32.h>

#undef WIN32_LEAN_AND_MEAN
#undef _WINSOCKAPI_
#undef NOMINMAX

// C++ Standard library
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <thread>
#include <unordered_map>

#endif // PCH_H
