#include "pch.hpp"
#include "ban.hpp"
#include "hardware_id.hpp"
#include "network.hpp"
#include "utils.hpp"


/**
 * \brief Ban current played
 * \param detection Information about what was detected
 * \return For potential future use, currently always returns true
 */
bool Ban(const DetectionInfo& detection) {

    Log("\nCHEAT FOUND: " + detection.cheat_name);

    // Get HWID info to be used as a UUID
    HardwareID hwid;

    nlohmann::json ban_info{ {"detection", {
        {"cheat", detection.cheat_name},
        {"scan", detection.scan_type},
        {"path", detection.cheat_path},
        {"blacklisted_domains", detection.blacklisted_domains},
        {"uuid", hwid.GetHash()}
    }} };

    // Fire and forget the ban message to the server
    CallAsync(Communication::SendData, ban_info);

    return true;
}
