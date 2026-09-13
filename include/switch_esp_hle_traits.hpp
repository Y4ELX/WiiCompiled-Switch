#pragma once

#include "abi_bridge.h"

// ESP_InitLib (PAL 0x801671D0). Pinned WiiCompiled does not open the Wii
// /dev/es device on the host; its native override simply reports success so the
// guest can continue through the title-services bootstrap.
template <>
struct KnownNativeCpuCall<0x801671D0u> {
    static constexpr bool kAvailable = true;

    static inline void Invoke(CpuContext* cpu) noexcept {
        if (cpu) {
            cpu->gpr[3] = 0u;
        }
    }
};
