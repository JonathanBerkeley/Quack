﻿#pragma once
#include "pch.hpp"


struct DnsEntry {
    std::wstring name{};
    int type{};
};

std::optional<std::vector<DnsEntry>> GetCachedDNSData(const bool cached_load);
std::optional<std::vector<DnsEntry>> CheckForBlacklistedDNSEntries();

void PrintDNSEntries(std::wostream& wos);