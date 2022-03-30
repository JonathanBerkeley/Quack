#pragma once
#include "pch.h"

#include "data.hpp"


[[nodiscard]] std::vector<HMODULE> EnumerateModules(data::ProcessInfo& pi);
bool VerifyModule(LPCWSTR source_file);
std::vector<unsigned long> PatternToByte(const std::string& signature);
std::uint8_t* PatternScan(const HMODULE module, const std::string& signature);
void ModuleScan(const data::ProcessInfo& context, const bool unsigned_only);
