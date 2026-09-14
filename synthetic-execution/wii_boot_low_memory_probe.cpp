#include "wii_boot_low_memory.hpp"

#include "memory_switch_slice.hpp"

#include <cstdint>

extern "C" __attribute__((noinline, used))
bool synthetic_wii_boot_low_memory_probe() noexcept {
    if (!Memory::IsInitialized()) {
        return false;
    }

    mkw::wii_boot_low_memory::seed_pinned_defaults();
    const auto layout = mkw::wii_boot_low_memory::pinned_layout();

    // These values are derived only from the pinned runtime memory contract:
    // 24 MiB MEM1, 128 MiB mapped MEM2, 128 KiB IOS/IPC reservations and a
    // 2 MiB runtime FST reservation. No Nintendo/game data is involved.
    return
        layout.mem1_size == 0x01800000u &&
        layout.mem1_arena_lo == 0x80399180u &&
        layout.mem1_arena_hi == 0x817F0520u &&
        layout.mem2_size == 0x08000000u &&
        layout.mem2_end == 0x97FE0000u &&
        layout.mem2_arena_lo == 0x90000800u &&
        layout.mem2_arena_hi == 0x97DC0000u &&
        layout.ipc_buffer_lo == 0x97FC0000u &&
        layout.ipc_buffer_hi == 0x97FE0000u &&
        layout.ios_reserved_lo == 0x97FE0000u &&
        layout.ios_reserved_hi == 0x98000000u &&
        mkw::wii_boot_low_memory::matches_pinned_defaults();
}
