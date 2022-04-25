#pragma once


namespace cfg {

    enum Scan : bool {
        DNS = true,
        Module = true,
        UnsignedModulesOnly = true,
        ExecutableMemory = true
    };

    constexpr double cpu_usage_threshold = 90.0;

}
