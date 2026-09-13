#pragma once

#include "abi_bridge.h"

// DVDLowClearCoverInterrupt (PAL 0x80166964). Pinned WiiCompiled does not
// emulate the Wii optical-drive cover interrupt on the host. The native
// override ignores the callback argument and reports the low-level request as
// accepted/ready immediately.
template <>
struct KnownNativeCpuCall<0x80166964u> {
    static constexpr bool kAvailable = true;

    static inline void Invoke(CpuContext* cpu) noexcept {
        if (cpu) {
            cpu->gpr[3] = 1u;
        }
    }
};
