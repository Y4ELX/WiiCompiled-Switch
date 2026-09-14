#pragma once

#include "abi_bridge.h"

extern "C" void mkw_switch_hle_vi_init(CpuContext* cpu) noexcept;

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
