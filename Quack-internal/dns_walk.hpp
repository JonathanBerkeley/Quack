#pragma once
#include "pch.h"


struct DnsEntry {
    std::wstring name{};
    int type{};
};

std::optional<std::vector<DnsEntry>> GetCachedDNSData();
std::optional<std::vector<DnsEntry>> CheckForBlacklistedDNSEntries();
