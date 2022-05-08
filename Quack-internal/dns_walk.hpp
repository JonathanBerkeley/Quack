#pragma once
#include "pch.hpp"


/**
 * \brief Represents an entry in the DNS table
 */
struct DnsEntry {
    std::wstring name{};
    int type{};
};

std::optional<std::vector<DnsEntry>> GetCachedDNSData(const bool cached_load);
std::optional<std::vector<DnsEntry>> CheckForBlacklistedDNSEntries();

void PrintDNSEntries(std::wostream& wos);
void DnsScan();
