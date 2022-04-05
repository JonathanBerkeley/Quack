#pragma once


namespace cfg {

    enum Config : bool {
        DNSScanning = true,
        ModuleSigScanning = true,
        UnsignedModulesOnly = true
    };

    constexpr double cpu_usage_threshold = 90.0;

}
