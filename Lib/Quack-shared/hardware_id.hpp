#pragma once
#include "hash-library/sha256.hpp"


/**
 * \brief Class for
 */
class HardwareID {
private:
    std::string raw_hwid;
    SHA256 hash;

    static std::optional<std::string> GetHWID();

    static std::optional<DWORD> DiskID();

    static std::optional<std::string> GpuID();

    static char16_t GetCpuID();
public:
    /**
     * \brief Creates an ID that is identifiable to a specific PC
     * Value is created on construction, and can be retrieved as a SHA256 hash through GetHash() or as a raw value through GetRawHWID()
     *
     * Much of the implementation was learned from https://github.com/medievalghoul/hwid-checker-mg
     */
    HardwareID();

    /**
     * \brief Gets std::string representation of SHA256 hash created around HardwareID
     *
     * Created to address privacy concerns around distribution of raw Hardware IDs
     * \return std::string representation of SHA256 hash of HardwareID
     */
    std::string GetHash();

    /**
     * \brief Gets std::string representation of various identifying serials concatenated together
     *
     * Not suitable for networking, as it contains potentially identifiable information encoded
     * \return std::string encoding of HardwareID
     */
    std::string GetRawHWID();
};
