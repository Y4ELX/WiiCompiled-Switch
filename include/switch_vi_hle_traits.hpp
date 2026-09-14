#pragma once

#include "abi_bridge.h"

extern "C" void mkw_switch_hle_vi_init(CpuContext* cpu) noexcept;
extern "C" void mkw_switch_hle_vi_set_black(CpuContext* cpu) noexcept;

template <>
struct KnownNativeCpuCall<0x801B94A4u> {
    static constexpr bool kAvailable = true;

    static inline void Invoke(CpuContext* cpu) noexcept {
        mkw_switch_hle_vi_init(cpu);
    }
};

template <>
struct KnownNativeCpuCall<0x801B9294u> {
    static constexpr bool kAvailable = true;

    static inline void Invoke(CpuContext* cpu) noexcept {
        mkw_switch_hle_vi_init(cpu);
    }
};

// VIGetDTVStatus (PAL 0x801BAD38). Pinned WiiCompiled deliberately skips the
// Wii VI MMIO read at 0xCC00206E and reports DTV as not ready / disabled.
// This boundary has no additional guest-memory, retrace or renderer effects.
template <>
struct KnownNativeCpuCall<0x801BAD38u> {
    static constexpr bool kAvailable = true;

    static inline void Invoke(CpuContext* cpu) noexcept {
        if (cpu) {
            cpu->gpr[3] = 0u;
        }
    }
};

template <>
struct KnownNativeCpuCall<0x801BAB2Cu> {
    static constexpr bool kAvailable = true;

    static inline void Invoke(CpuContext* cpu) noexcept {
        mkw_switch_hle_vi_set_black(cpu);
    }
};
