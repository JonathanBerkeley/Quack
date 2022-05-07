#include "pch.hpp"
#include "cpu_counter.hpp"


// Cpu counter implementation

#pragma region ConstructorDestructor
CpuCounter::CpuCounter() {
    PdhOpenQuery(nullptr, 0, &cpu_query);
    PdhAddEnglishCounter(cpu_query, L"\\Processor(_Total)\\% Processor Time", 0, &cpu_counter);
    PdhCollectQueryData(cpu_query);
}

CpuCounter::~CpuCounter() {
    // Cleanup
    PdhRemoveCounter(cpu_counter);
    PdhCloseQuery(cpu_query);
}
#pragma endregion

#pragma region CopyMoveAssign
CpuCounter::CpuCounter(const CpuCounter& other) = default;

CpuCounter::CpuCounter(CpuCounter&& other) noexcept :
    cpu_query(other.cpu_query),
    cpu_counter(other.cpu_counter) {}

CpuCounter& CpuCounter::operator=(const CpuCounter& other) {
    if (this == &other)
        return *this;
    cpu_query = other.cpu_query;
    cpu_counter = other.cpu_counter;
    return *this;
}

CpuCounter& CpuCounter::operator=(CpuCounter&& other) noexcept {
    if (this == &other)
        return *this;
    cpu_query = other.cpu_query;
    cpu_counter = other.cpu_counter;
    return *this;
}
#pragma endregion

/**
 * \brief Queries PDH for cpu usage statistics
 * \return Returns double representing total CPU usage as a percent
 */
double CpuCounter::operator()() const {
    PDH_FMT_COUNTERVALUE counter_value;
    PdhCollectQueryData(cpu_query);
    PdhGetFormattedCounterValue(cpu_counter, PDH_FMT_DOUBLE, nullptr, &counter_value);

    return counter_value.doubleValue;
}
