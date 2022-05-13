#include "pch.hpp"
#include "flashpoint.hpp"
#include "hardware_id.hpp"


extern "C" {
    void __declspec(dllexport) GetHWID(char* message) {
        HardwareID hwid;
        const std::string data = hwid.GetHash();

        for (auto i = 0u; i < data.length(); ++i) {
            message[i] = data[i];
        }
    }
}
