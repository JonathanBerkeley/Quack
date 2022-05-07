#pragma once
#include "pch.hpp"
#include "context.hpp"
#include "identification.hpp"


/**
 * \brief Contains information that will be transmitted with heartbeats
 */
struct HeartbeatInfo {
    std::string hwid;
    std::string username;

    Hashes arp_hashes;
    unsigned risk_factor;
};

[[nodiscard]]
bool Heartbeat(const Context& ctx, const HeartbeatInfo& heartbeat_info);
