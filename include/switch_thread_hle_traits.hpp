#pragma once

#include "abi_bridge.h"
#include "memory.h"

#include <cstdint>

// OSGetCurrentThread (PAL 0x801A98B0). Pinned WiiCompiled registers this as a
// native function. Its complete guest-visible behavior is to return the running
// guest thread/context pointer from low memory at 0x800000E4, or null if the
// guest word cannot be read.
template <>
struct KnownNativeCpuCall<0x801A98B0u> {
    static constexpr bool kAvailable = true;

    static inline void Invoke(CpuContext* cpu) noexcept {
        if (!cpu) {
            return;
        }

        constexpr std::uint32_t kOSRunningContextAddr = 0x800000E4u;
        try {
            cpu->gpr[3] = Memory::Read32(kOSRunningContextAddr);
        } catch (...) {
            cpu->gpr[3] = 0u;
        }
    }
};
