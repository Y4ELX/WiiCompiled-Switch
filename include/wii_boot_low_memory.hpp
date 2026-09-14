#pragma once

#include <cstdint>

namespace mkw::wii_boot_low_memory {

// PAL RMCP01 low-memory defaults mirrored from the pinned WiiCompiled
// SystemBridge startup contract. These are runtime/platform values only; they
// contain no game payload or generated translated data.
struct Layout {
    std::uint32_t mem1_size;
    std::uint32_t mem1_arena_lo;
    std::uint32_t mem1_arena_hi;
    std::uint32_t mem2_size;
    std::uint32_t mem2_end;
    std::uint32_t mem2_arena_lo;
    std::uint32_t mem2_arena_hi;
    std::uint32_t ipc_buffer_lo;
    std::uint32_t ipc_buffer_hi;
    std::uint32_t ios_reserved_lo;
    std::uint32_t ios_reserved_hi;
};

Layout pinned_layout() noexcept;
void seed_pinned_defaults() noexcept;
bool matches_pinned_defaults() noexcept;

} // namespace mkw::wii_boot_low_memory
