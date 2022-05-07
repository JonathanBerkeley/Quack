#pragma once

namespace cfg {

    enum Scan : bool {
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

    constexpr double cpu_usage_threshold = 90.0;
}
