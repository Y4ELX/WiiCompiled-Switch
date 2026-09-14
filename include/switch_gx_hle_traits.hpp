#pragma once

#include "abi_bridge.h"

extern "C" void mkw_switch_hle_gx_init(CpuContext* cpu) noexcept;

template <>
struct KnownNativeCpuCall<0x8016B850u> {
    static constexpr bool kAvailable = true;

    static inline void Invoke(CpuContext* cpu) noexcept {
        mkw_switch_hle_gx_init(cpu);
    }
};
