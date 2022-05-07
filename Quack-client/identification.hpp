#pragma once


using Hashes = std::vector<std::string>;
[[nodiscard]]
std::optional<std::vector<std::string>> GetArpMacHashes();
