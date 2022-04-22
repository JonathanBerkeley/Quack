#pragma once
#include "pch.hpp"


using ProcessEntry = std::unordered_map<DWORD, std::wstring>;
std::optional<ProcessEntry> FindProcesses(const std::vector<std::wstring>& blacklist);

unsigned KillBlacklistedProcesses(const std::vector<std::wstring>& blacklist);
