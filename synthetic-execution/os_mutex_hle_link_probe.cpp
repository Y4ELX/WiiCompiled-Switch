#include "abi_bridge.h"
#include "switch_mutex_hle_traits.hpp"

#include <cstdint>

static_assert(KnownNativeCpuCall<0x801A7EE4u>::kAvailable);
static_assert(KnownNativeCpuCall<0x801A7FC0u>::kAvailable);

// Nintendo-data-free compile/link coverage for the blocker-driven OSMutex HLE.
// Null mutex pointers exercise the safe early return while still instantiating
// the exact static direct-dispatch templates used by real translated shards.
extern "C" __attribute__((noinline, used)) void synthetic_os_mutex_hle_link_probe(
    CpuContext* cpu) {
    if (!cpu) {
        return;
    }

    const std::uint32_t savedR3 = cpu->gpr[3];
    cpu->gpr[3] = 0u;
    InvokeDirectCpu<0x801A7EE4u>(cpu);
    InvokeDirectCpu<0x801A7FC0u>(cpu);
    cpu->gpr[3] = savedR3;
}
