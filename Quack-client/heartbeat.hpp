#pragma once

#include "context.hpp"
#include "identification.hpp"


struct HeartbeatInfo {
    std::string hwid;
    std::string username;

    Hashes arp_hashes;
    unsigned risk_factor;
};

[[nodiscard]]
bool Heartbeat(const Context& ctx, const HeartbeatInfo& heartbeat_info);
