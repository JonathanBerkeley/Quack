#pragma once


/**
 * \brief Represents information to be transmitted to remote server when ban-worthy behaviour
 * has been detected
 */
struct DetectionInfo {
    std::string cheat_name;
    std::string scan_type;
    std::wstring cheat_path;

    std::vector<std::string> blacklisted_domains;
};

bool Ban(const DetectionInfo& detection);
