#pragma once


namespace cfg {

    enum Scan : bool {
        DNS = true,
        Module = true,
        UnsignedModulesOnly = true,
        ExecutableMemory = true
    };

    enum ExitCode : int {
        Success,
        DebuggerPresent,
        NoHeartbeat
    };

    constexpr double cpu_usage_threshold = 90.0;

}
