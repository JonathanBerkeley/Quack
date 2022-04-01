#include "pch.hpp"
#include "dns_walk.hpp"
#include "utils.hpp"

#include "WinDNS.h"


typedef struct DnsData {
    DnsData* next;
    PWSTR name;
    WORD type;
    WORD data_length;
    DWORD flags;
} *DnsTablePtr;


using DnsTableCachePtr = int (WINAPI*)(DnsTablePtr* dns_table_ptr);
using DnsApiFreePtr = void (WINAPI*)(PVOID p_data);
using DnsFreePtr = void (WINAPI*)(PVOID p_data, DNS_FREE_TYPE free_type);

using DnsEntries = std::optional<std::vector<DnsEntry>>;
namespace ranges = std::ranges;


// ReSharper disable CppLocalVariableMayBeConst (Buggy with WinAPI typedefs)
/**
 * \brief Walk the DNS cache using undocumented DNSAPI function
 * Will cause DNSAPI.dll to be loaded temporarily
 *
 * Logic based on https://github.com/malcomvetter/DnsCache
 * Improved with advice from https://stackoverflow.com/a/31892801
 * \param cached_load Whether or not to cache the DLL load attempt.
 * Better for multiple runs, so that DNSAPI.dll is not continually reloaded.
 * However, using cached_load will mean that DNSAPI.dll will remain loaded even after
 * this function is finished.
 * \return Vector containing cached DnsEntries on success
 */
DnsEntries GetCachedDNSData(const bool cached_load) {
    std::vector<DnsEntry> entries{};

    DnsTablePtr table_entry = nullptr;

    HMODULE dns_lib = cached_load
        ? CachedLoadLibrary(L"DNSAPI.dll")
        : LoadLibrary(L"DNSAPI.dll");

    if (dns_lib == nullptr)
        return std::nullopt;
    
    // Obtain pointer to undocumented DnsGetCacheDataTable function in DNSAPI
    FARPROC DnsGetCacheDataTableFunc = GetProcAddress(dns_lib, "DnsGetCacheDataTable");
    const auto DnsGetCacheDataTable = reinterpret_cast<DnsTableCachePtr>(DnsGetCacheDataTableFunc);

    // Obtain pointer to DnsFree function in DNSAPI
    FARPROC DnsFreeFunc = GetProcAddress(dns_lib, "DnsFree");
    const auto DnsFree = reinterpret_cast<DnsFreePtr>(DnsFreeFunc);

    // Gets pointer to beginning of table
    DnsGetCacheDataTable(&table_entry);

    if (table_entry == nullptr) {
        DnsFree(table_entry, DnsFreeFlat);
        return std::nullopt;
    }

    // Enumerate the cache
    while (table_entry) {
        // Store copy of entry that is found
        DnsEntry this_entry;
        this_entry.name = std::wstring{ table_entry->name };
        this_entry.type = table_entry->type;
        entries.emplace_back(this_entry);

        // Cleanup
        DnsFree(table_entry->name, DnsFreeFlat);
        PVOID previous_p_entry = table_entry;

        // Go to next entry
        table_entry = table_entry->next;

        DnsFree(previous_p_entry, DnsFreeFlat);
    }

    // todo: Verify no leaks possible
    // todo: Keep dns_lib loaded if repeatedly searching ?

    // Cleanup
    if (!cached_load) {
        FreeLibrary(dns_lib);
    }

    return entries;
}
// ReSharper restore CppLocalVariableMayBeConst


std::vector<std::wstring> GetBlacklistedEntries() {
    // todo: Network blacklisted entries

    std::vector<std::wstring> blacklist{
        // Examples only
        L"client.aimware.net",
        L"aimware.net",
        L"cdn.aimware.net"
    };

    return blacklist;
}


/**
 * \brief Searches entries in the DNS cache for blacklisted domains
 * \return List of entries that matched the blacklist, or empty
 */
DnsEntries CheckForBlacklistedDNSEntries() {
    std::vector<DnsEntry> matches{};

    auto blacklist = GetBlacklistedEntries();

    const auto dns_cache = GetCachedDNSData(true);
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


/**
 * \brief Prints DNS entries in a human readable format to supplied wide output stream
 * \param wos References to wide output stream
 */
void PrintDNSEntries(std::wostream& wos) {
    const auto dns_cache = GetCachedDNSData(true);

    if (!dns_cache) {
        wos << L"DNS search failed.\n";
        return;
    }

    // Map dns flag values to printable names
    // https://docs.microsoft.com/en-us/windows/win32/dns/dns-constants
    const std::unordered_map<int, std::wstring> dns_mappings{
        {DNS_TYPE_A, L"A"},
        {DNS_TYPE_NS, L"NS"},
        {DNS_TYPE_MD, L"MD"},
        {DNS_TYPE_MF, L"MF"},
        {DNS_TYPE_CNAME, L"CNAME"},
        {DNS_TYPE_SOA, L"SOA"},
        {DNS_TYPE_MB, L"MB"},
        {DNS_TYPE_MG, L"MG"},
        {DNS_TYPE_MR, L"MR"},
        {DNS_TYPE_NULL, L"NULL"},
        {DNS_TYPE_WKS, L"WKS"},
        {DNS_TYPE_PTR, L"PTR"},
        {DNS_TYPE_HINFO, L"HINFO"},
        {DNS_TYPE_MINFO, L"MINFO"},
        {DNS_TYPE_MX, L"MX"},
        {DNS_TYPE_TEXT, L"TEXT"},
        {DNS_TYPE_RP, L"RP"},
        {DNS_TYPE_AFSDB, L"AFSDB"},
        {DNS_TYPE_X25, L"X25"},
        {DNS_TYPE_ISDN, L"ISDN"},
        {DNS_TYPE_RT, L"RT"},
        {DNS_TYPE_NSAP, L"NSAP"},
        {DNS_TYPE_NSAPPTR, L"NSAPPTR"},
        {DNS_TYPE_SIG, L"SIG"},
        {DNS_TYPE_KEY, L"KEY"},
        {DNS_TYPE_PX, L"PX"},
        {DNS_TYPE_GPOS, L"GPOS"},
        {DNS_TYPE_AAAA, L"AAAA"},
        {DNS_TYPE_LOC, L"LOC"},
        {DNS_TYPE_NXT, L"NXT"},
        {DNS_TYPE_EID, L"EID"},
        {DNS_TYPE_NIMLOC, L"NIMLOC"},
        {DNS_TYPE_SRV, L"SRV"},
        {DNS_TYPE_ATMA, L"ATMA"},
        {DNS_TYPE_NAPTR, L"NAPTR"},
        {DNS_TYPE_KX, L"KX"},
        {DNS_TYPE_CERT, L"CERT"},
        {DNS_TYPE_A6, L"A6"},
        {DNS_TYPE_DNAME, L"DNAME"},
        {DNS_TYPE_SINK, L"SINK"},
        {DNS_TYPE_OPT, L"OPT"},
        {DNS_TYPE_DS, L"DS"},
        {DNS_TYPE_RRSIG, L"RRSIG"},
        {DNS_TYPE_NSEC, L"NSEC"},
        {DNS_TYPE_DNSKEY, L"DNSKEY"},
        {DNS_TYPE_DHCID, L"DHCID"},
        {DNS_TYPE_UINFO, L"UINFO"},
        {DNS_TYPE_UID, L"UID"},
        {DNS_TYPE_GID, L"GID"},
        {DNS_TYPE_UNSPEC, L"UNSPEC"},
        {DNS_TYPE_ADDRS, L"ADDRS"},
        {DNS_TYPE_TKEY, L"TKEY"},
        {DNS_TYPE_TSIG, L"TSIG"},
        {DNS_TYPE_IXFR, L"IXFR"},
        {DNS_TYPE_AXFR, L"AXFR"},
        {DNS_TYPE_MAILB, L"MAILB"},
        {DNS_TYPE_MAILA, L"MAILA"},
        {DNS_TYPE_ALL, L"ALL"},
        {DNS_TYPE_ANY, L"ANY"},
        {DNS_TYPE_WINS, L"WINS"},
        {DNS_TYPE_WINSR, L"WINSR"},
        {DNS_TYPE_NBSTAT, L"NBSTAT"}
    };

    int count = 0;
    for (const auto& [name, type] : dns_cache.value()) {
        try {
            ++count;
            wos << dns_mappings.at(type) << L" : " << name << L'\n';
        }
        catch (const std::out_of_range&) {
            wos << L"Unknown : " << name << L'\n';
        }
    }

    wos << L"DNS Records found: "  << count << '\n';
}
