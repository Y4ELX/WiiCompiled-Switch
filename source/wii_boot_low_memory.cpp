#include "wii_boot_low_memory.hpp"

#include "memory_switch_slice.hpp"

#include <cstddef>
#include <cstdint>

namespace mkw::wii_boot_low_memory {
namespace {

constexpr std::uint32_t kMem1ArenaLoDefault = 0x80399180u;
constexpr std::uint32_t kMem1ArenaHiDefault = 0x817F0520u;
constexpr std::uint32_t kMem2LoFloor = 0x90000800u;
constexpr std::uint32_t kDvdFstReserveSize = 0x00200000u;
constexpr std::uint32_t kIpcArenaSize = 0x00020000u;
constexpr std::uint32_t kIosReservedSize = 0x00020000u;

struct SeedEntry {
    std::uint32_t address;
    std::uint32_t value;
};

bool mapped_word(std::uint32_t address) noexcept {
    return Memory::IsInitialized() && Memory::Contains(address, sizeof(std::uint32_t));
}

void write_word(std::uint32_t address, std::uint32_t value) noexcept {
    if (mapped_word(address)) {
        Memory::Write32(address, value);
    }
}

bool matches_word(std::uint32_t address, std::uint32_t value) noexcept {
    return mapped_word(address) && Memory::Read32(address) == value;
}

} // namespace

Layout pinned_layout() noexcept {
    const std::uint32_t mem1Size = static_cast<std::uint32_t>(Memory::kMem1Size);
    const std::uint32_t mem2Size = static_cast<std::uint32_t>(Memory::kMem2Size);
    const std::uint32_t physicalMem2End = Memory::kMem2CachedBase + mem2Size;
    const std::uint32_t iosReservedLo = physicalMem2End - kIosReservedSize;
    const std::uint32_t ipcBufferHi = iosReservedLo;
    const std::uint32_t ipcBufferLo = ipcBufferHi - kIpcArenaSize;
    const std::uint32_t mem2ArenaHi = ipcBufferLo - kDvdFstReserveSize;

    return {
        .mem1_size = mem1Size,
        .mem1_arena_lo = kMem1ArenaLoDefault,
        .mem1_arena_hi = kMem1ArenaHiDefault,
        .mem2_size = mem2Size,
        .mem2_end = iosReservedLo,
        .mem2_arena_lo = kMem2LoFloor,
        .mem2_arena_hi = mem2ArenaHi,
        .ipc_buffer_lo = ipcBufferLo,
        .ipc_buffer_hi = ipcBufferHi,
        .ios_reserved_lo = iosReservedLo,
        .ios_reserved_hi = physicalMem2End,
    };
}

void seed_pinned_defaults() noexcept {
    if (!Memory::IsInitialized()) {
        return;
    }

    const Layout layout = pinned_layout();
    const std::uint32_t mem1End = Memory::kMem1CachedBase + layout.mem1_size;

    // This is the memory/arena subset of pinned WiiCompiled
    // SystemBridge::SeedLowMemDefaults(). The pin runs it after Memory::Init
    // and before generated data-section initialization. All of these entries
    // are forced there as loader/IOS-provided low-memory state.
    const SeedEntry entries[] = {
        {0x80000028u, layout.mem1_size},
        {0x80000030u, layout.mem1_arena_lo},
        {0x80000034u, layout.mem1_arena_hi},
        {0x800000F0u, layout.mem1_size},
        {0x80003100u, layout.mem1_size},
        {0x80003104u, layout.mem1_size},
        {0x80003108u, mem1End},
        {0x8000310Cu, layout.mem1_arena_lo},
        {0x80003110u, layout.mem1_arena_hi},
        {0x80003118u, layout.mem2_size},
        {0x8000311Cu, layout.mem2_size},
        {0x80003120u, layout.mem2_end},
        {0x80003124u, layout.mem2_arena_lo},
        {0x80003128u, layout.mem2_arena_hi},
        {0x80003130u, layout.ipc_buffer_lo},
        {0x80003134u, layout.ipc_buffer_hi},
        {0x80003138u, 0x00000002u},
        {0x80003148u, layout.ios_reserved_lo},
        {0x8000314Cu, layout.ios_reserved_hi},
    };

    for (const SeedEntry& entry : entries) {
        write_word(entry.address, entry.value);
    }
}

bool matches_pinned_defaults() noexcept {
    if (!Memory::IsInitialized()) {
        return false;
    }

    const Layout layout = pinned_layout();
    const std::uint32_t mem1End = Memory::kMem1CachedBase + layout.mem1_size;
    return matches_word(0x80000028u, layout.mem1_size) &&
           matches_word(0x80000030u, layout.mem1_arena_lo) &&
           matches_word(0x80000034u, layout.mem1_arena_hi) &&
           matches_word(0x800000F0u, layout.mem1_size) &&
           matches_word(0x80003100u, layout.mem1_size) &&
           matches_word(0x80003104u, layout.mem1_size) &&
           matches_word(0x80003108u, mem1End) &&
           matches_word(0x8000310Cu, layout.mem1_arena_lo) &&
           matches_word(0x80003110u, layout.mem1_arena_hi) &&
           matches_word(0x80003118u, layout.mem2_size) &&
           matches_word(0x8000311Cu, layout.mem2_size) &&
           matches_word(0x80003120u, layout.mem2_end) &&
           matches_word(0x80003124u, layout.mem2_arena_lo) &&
           matches_word(0x80003128u, layout.mem2_arena_hi) &&
           matches_word(0x80003130u, layout.ipc_buffer_lo) &&
           matches_word(0x80003134u, layout.ipc_buffer_hi) &&
           matches_word(0x80003138u, 0x00000002u) &&
           matches_word(0x80003148u, layout.ios_reserved_lo) &&
           matches_word(0x8000314Cu, layout.ios_reserved_hi);
}

} // namespace mkw::wii_boot_low_memory
