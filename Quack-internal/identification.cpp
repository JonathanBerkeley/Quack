#include "pch.hpp"

#include "d3d9.h"
#pragma comment(lib, "d3d9.lib")

void GetGPUID() {

    // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getsystemfirmwaretable
    auto smbios = GetSystemFirmwareTable(
        static_cast<int>('RSMB'),
        0,
        nullptr,
        0
    );

    auto dummy_object = Direct3DCreate9(D3D_SDK_VERSION);
    auto adapter_count = dummy_object->GetAdapterCount();
    // todo: Hwid
}