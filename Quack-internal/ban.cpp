#include "pch.hpp"
#include "ban.hpp"

#include "hardware_id.hpp"
#include "network.hpp"
#include "utils.hpp"


bool Ban(const DetectionInfo& detection) {

    Log("\nCHEAT FOUND: " + detection.cheat_name);

    // Get HWID info to be used as a UUID
    HardwareID hwid;

    nlohmann::json ban_info{ {"detection", {
        {"cheat", detection.cheat_name},
        {"scan", detection.scan_type},
        {"path", detection.cheat_path},
        {"uuid", hwid.GetHash()}
    }} };

    // Fire and forget the ban message to the server
    CallAsync(Communication::SendData, ban_info);

    return true;
}
