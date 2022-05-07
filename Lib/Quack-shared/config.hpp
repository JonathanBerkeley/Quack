#pragma once


/**
 * \brief Config of this build, values changed here will enable or disable functionality
 * True indicates feature is enabled, false indicates disabled
 */
namespace cfg {

    enum Scan : bool {
        CpuThrottle = true,
        DNS = true,
        Module = true,
        UnsignedModulesOnly = true,
        ExecutableMemory = true
    };

    enum Features : bool {
        ARP = true,
        GpuID = true,
        CpuID = true,
        DiskID = true
    };

    enum ExitCode : int {
        Success,
        DebuggerPresent,
        NoHeartbeat,
        BlacklistedProgram
    };

    // Percent to throttle CPU usage at, if Scan::CpuThrottle is enabled
    constexpr double cpu_usage_threshold = 90.0;
}
