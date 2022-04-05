#pragma once
#include "pch.hpp"

#pragma comment(lib, "pdh.lib")

/**
 * \brief RAII Wrapper for querying cpu usage through Pdh WinAPI
 *
 * WinAPI details: https://docs.microsoft.com/en-us/windows/win32/api/pdh/
 */
class CpuCounter {
private:
    PDH_HQUERY cpu_query{};
    PDH_HCOUNTER cpu_counter{};

public:
    CpuCounter();
    ~CpuCounter();

    CpuCounter(const CpuCounter& other);
    CpuCounter(CpuCounter&& other) noexcept;
    CpuCounter& operator=(const CpuCounter& other);
    CpuCounter& operator=(CpuCounter&& other) noexcept;

    [[nodiscard]]
    double operator()() const;
};
