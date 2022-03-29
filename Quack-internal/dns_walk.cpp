#include "pch.h"
#include "dns_walk.h"


typedef struct DnsData {
    DnsData* next;
    PWSTR name;
    WORD type;
    WORD data_length;
    DWORD flags;
} dns_table, * p_dns_table;

using DnsTableCache = int (WINAPI*)(p_dns_table);
using DnsEntries = std::optional<std::vector<DnsEntry>>;
namespace ranges = std::ranges;


/**
 * \brief Walk the DNS cache using undocumented DNSAPI function
 * Will cause DNSAPI.dll to be loaded temporarily
 *
 * Logic based on https://github.com/malcomvetter/DnsCache
 * \return Vector containing cached DnsEntries on success
 */
DnsEntries GetCachedDNSData() {
    std::vector<DnsEntry> entries{};

    auto entry = static_cast<p_dns_table>(
        std::malloc(sizeof dns_table)
        );
    const auto dns_lib = LoadLibrary(L"DNSAPI.dll");

    if (dns_lib == nullptr)
        return std::nullopt;

    FARPROC proc = GetProcAddress(dns_lib, "DnsGetCacheDataTable");
    const auto get_cached_dns_table = reinterpret_cast<DnsTableCache>(proc);

    get_cached_dns_table(entry);

    if (entry == nullptr)
        return std::nullopt;

    entry = entry->next;
    while (entry) {
        DnsEntry this_entry;
        this_entry.name = std::wstring{ entry->name };
        this_entry.type = entry->type;
        entries.emplace_back(this_entry);

        entry = entry->next;
    }
    FreeLibrary(dns_lib);
    std::free(entry);

    return entries;
}


/**
 * \brief Searches entries in the DNS cache for blacklisted domains
 * \return List of entries that matched the blacklist, or empty
 */
DnsEntries CheckForBlacklistedDNSEntries() {
    std::vector<DnsEntry> matches{};

    // todo: network blacklisted DNS entries
    std::vector<std::wstring> blacklist{
        L"www.facebook.com" // Example only
    };

    const auto dns_cache = GetCachedDNSData();
    if (!dns_cache)
        return std::nullopt;

    for (const auto& entry : dns_cache.value()) {
        if (ranges::find(blacklist, entry.name) != blacklist.end()) {
            matches.emplace_back(entry);
        }
    }

    // Return the entries, or empty if there were none
    return matches.empty() ? std::nullopt : DnsEntries{ matches };
}
