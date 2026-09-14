#if (defined(MKW_LOCAL_FUNCTION_EXECUTION) && MKW_LOCAL_FUNCTION_EXECUTION) || \
    (defined(MKW_SYNTHETIC_EXECUTION) && MKW_SYNTHETIC_EXECUTION)

#include "abi_bridge.h"
#include "memory.h"

#include <atomic>
#include <cstdint>

namespace {

constexpr std::uint32_t kViInitializedFlagAddr = 0x80386B38u;
constexpr std::uint32_t kViTimingGuardAddr = 0x80386B44u;
constexpr std::uint32_t kViTvFormatAddr = 0x80386BA8u;
constexpr std::uint32_t kViRenderWidthAddr = 0x80350864u;
constexpr std::uint32_t kViRenderHeightAddr = 0x80350866u;
constexpr std::uint32_t kViXfbWidthAddr = 0x80350872u;
constexpr std::uint32_t kViXfbHeightAddr = 0x8035087Cu;
constexpr std::uint32_t kViRetraceCountAddr = 0x80386BE4u;
constexpr std::uint32_t kViPreRetraceCallbackAddr = 0x80386BB8u;
constexpr std::uint32_t kViPostRetraceCallbackAddr = 0x80386BB4u;
constexpr std::uint32_t kViNextFrameBufferAddr = 0x80386BA0u;
constexpr std::uint32_t kViNextFrameBufferHwAddr = 0x80350890u;

std::atomic<bool> g_viInitialized{false};

void Write8IfMapped(std::uint32_t address, std::uint8_t value) noexcept {
    if (Memory::Contains(address, 1u)) {
        Memory::Write8(address, value);
    }
}

void Write16IfMapped(std::uint32_t address, std::uint16_t value) noexcept {
    if (Memory::Contains(address, 2u)) {
        Memory::Write16(address, value);
    }
}

void Write32IfMapped(std::uint32_t address, std::uint32_t value) noexcept {
    if (Memory::Contains(address, 4u)) {
        Memory::Write32(address, value);
    }
}

} // namespace

extern "C" void mkw_switch_hle_vi_init(CpuContext* cpu) noexcept {
    if (cpu) {
        cpu->gpr[3] = 0u;
    }

    if (!Memory::IsInitialized()) {
        return;
    }

    bool expected = false;
    if (!g_viInitialized.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        return;
    }

    // Pinned WiiCompiled's VIInit/__VIInit HLE skips Wii VI MMIO and seeds the
    // guest-visible defaults from ViState::EnsureInitializedLocked(). Keep the
    // Switch fast-track headless: this is VI bookkeeping only, not a presenter.
    Write8IfMapped(kViInitializedFlagAddr, 1u);
    Write8IfMapped(kViTimingGuardAddr, 1u);
    Write32IfMapped(kViTvFormatAddr, 0u);
    Write16IfMapped(kViRenderWidthAddr, 640u);
    Write16IfMapped(kViRenderHeightAddr, 480u);
    Write16IfMapped(kViXfbWidthAddr, 640u);
    Write16IfMapped(kViXfbHeightAddr, 480u);
    Write32IfMapped(kViRetraceCountAddr, 0u);
    Write32IfMapped(kViPreRetraceCallbackAddr, 0u);
    Write32IfMapped(kViPostRetraceCallbackAddr, 0u);
    Write32IfMapped(kViNextFrameBufferAddr, 0u);
    Write32IfMapped(kViNextFrameBufferHwAddr, 0u);
}

#endif
