#pragma once

#include "abi_bridge.h"
#include "memory.h"

#include <cstdint>

namespace mkw::dvd_hle_detail {

inline bool IsGameCodeByte(std::uint8_t value) noexcept {
    return (value >= static_cast<std::uint8_t>('A') &&
            value <= static_cast<std::uint8_t>('Z')) ||
           (value >= static_cast<std::uint8_t>('0') &&
            value <= static_cast<std::uint8_t>('9'));
}

inline std::uint32_t CurrentDiscGameCode() noexcept {
    constexpr std::uint32_t kPalFallback = 0x524D4350u; // RMCP
    constexpr std::uint32_t kDiscHeader = 0x80000000u;

    if (!Memory::Contains(kDiscHeader, 4u)) {
        return kPalFallback;
    }

    const std::uint32_t value = Memory::Read32(kDiscHeader);
    for (int shift = 24; shift >= 0; shift -= 8) {
        const auto byte = static_cast<std::uint8_t>((value >> shift) & 0xFFu);
        if (!IsGameCodeByte(byte)) {
            return kPalFallback;
        }
    }
    return value;
}

inline bool HasPublishedFst() noexcept {
    constexpr std::uint32_t kFstAddressLowMem = 0x80000038u;
    constexpr std::uint32_t kFstSizeLowMem = 0x8000003Cu;
    constexpr std::uint32_t kMaxRuntimeFstSize = 0x00200000u;

    if (!Memory::Contains(kFstAddressLowMem, 8u)) {
        return false;
    }

    const std::uint32_t fstAddress = Memory::Read32(kFstAddressLowMem);
    const std::uint32_t fstSize = Memory::Read32(kFstSizeLowMem);
    if (fstAddress == 0u || fstSize < 12u || fstSize > kMaxRuntimeFstSize ||
        !Memory::Contains(fstAddress, fstSize)) {
        return false;
    }

    const std::uint32_t rootWord = Memory::Read32(fstAddress);
    const std::uint32_t entryCount = Memory::Read32(fstAddress + 8u);
    return (rootWord & 0xFF000000u) == 0x01000000u &&
           entryCount != 0u && entryCount <= 0x00010000u;
}

inline bool GuestBootstrapRangesAvailable() noexcept {
    return Memory::Contains(0x80000000u, 7u) &&
           Memory::Contains(0x80343230u, 0x20u) &&
           Memory::Contains(0x803434E0u, 0x80u) &&
           Memory::Contains(0x80386664u, 0xC2u);
}

inline void InitializeWaitingQueues() noexcept {
    constexpr std::uint32_t kQueueBase = 0x80343230u;
    for (std::uint32_t i = 0; i < 4u; ++i) {
        const std::uint32_t queue = kQueueBase + i * 8u;
        Memory::Write32(queue + 0u, queue);
        Memory::Write32(queue + 4u, queue);
    }
}

inline void InitializeCancelState() noexcept {
    InitializeWaitingQueues();

    Memory::Write32(0x80386664u, 0u); // Canceling
    Memory::Write32(0x80386668u, 0u); // ResumeFromHere
    Memory::Write32(0x80386670u, 0u); // PausingFlag
    Memory::Write32(0x8038667Cu, 1u); // CancelAllSync complete
    Memory::Write32(0x803866A8u, 1u); // PrepareReset complete
    Memory::Write32(0x803866F0u, 0u); // executing command block
}

inline void InitializeContexts() noexcept {
    constexpr std::uint32_t kContextBase = 0x803434E0u;
    constexpr std::uint32_t kContextStride = 0x20u;
    constexpr std::uint32_t kMagicValue = 0xFEEBDAEDu;

    for (std::uint32_t i = 0; i < 4u; ++i) {
        const std::uint32_t context = kContextBase + i * kContextStride;
        Memory::Write32(context + 0x0Cu, kMagicValue);
        Memory::Write32(context + 0x10u, i);
    }
}

inline void InitializeDiscHeader() noexcept {
    constexpr std::uint32_t kDiscHeader = 0x80000000u;
    Memory::Write32(kDiscHeader + 0x00u, CurrentDiscGameCode());
    Memory::Write16(kDiscHeader + 0x04u, 0x3031u); // maker code "01"
    Memory::Write8(kDiscHeader + 0x06u, 0x01u);    // disc number 1
}

} // namespace mkw::dvd_hle_detail

// DVDInit (PAL 0x8015EA1C). Pinned WiiCompiled replaces the Wii drive/IOS
// bootstrap with host-side DVD setup while still publishing substantial guest
// state: DVD flags, cancel/wait queues, context sentinels, the low-memory disc
// header, a runtime FST, then translated __DVDFSInit.
//
// The Switch fast-track does not yet have a trustworthy host DVD/FST source, so
// it mirrors every startup-visible guest write but deliberately does not invent
// an FST. If low memory already contains a structurally valid FST, dispatch the
// translated __DVDFSInit exactly like upstream; otherwise leave FST publication
// for the dedicated DVD-storage bridge instead of fabricating game data.
template <>
struct KnownNativeCpuCall<0x8015EA1Cu> {
    static constexpr bool kAvailable = true;

    static inline void Invoke(CpuContext* cpu) noexcept {
        if (!cpu || !mkw::dvd_hle_detail::GuestBootstrapRangesAvailable()) {
            return;
        }

        constexpr std::uint32_t kDvdInitCalled = 0x803866A0u;
        if (Memory::Read8(kDvdInitCalled) != 0u) {
            return;
        }

        static bool initializing = false;
        if (initializing) {
            return;
        }
        initializing = true;

        Memory::Write8(0x80386724u, 1u);  // contexts initialized
        Memory::Write8(0x80386725u, 1u);  // low init called
        Memory::Write32(0x80386720u, 0u); // current context index

        mkw::dvd_hle_detail::InitializeCancelState();
        mkw::dvd_hle_detail::InitializeContexts();
        mkw::dvd_hle_detail::InitializeDiscHeader();

        if (mkw::dvd_hle_detail::HasPublishedFst()) {
            constexpr std::uint32_t kDvdFsInitAddress = 0x8015DF1Cu;
            InvokeIndirectCpu(kDvdFsInitAddress, cpu);
        }

        Memory::Write8(kDvdInitCalled, 1u);
        initializing = false;
    }
};
