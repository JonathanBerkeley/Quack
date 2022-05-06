#pragma once
#include "hash-library/sha256.hpp"


using Hashes = std::vector<std::string>;
[[nodiscard]]
std::optional<std::vector<std::string>> GetArpMacHashes();
