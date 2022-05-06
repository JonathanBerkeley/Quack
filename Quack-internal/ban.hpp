#pragma once


struct DetectionInfo {
    std::string cheat_name;
    std::string scan_type;
    std::wstring cheat_path;
};

bool Ban(const DetectionInfo& detection);
