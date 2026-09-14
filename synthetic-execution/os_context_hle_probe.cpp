#include "abi_bridge.h"
#include "switch_thread_hle_traits.hpp"

static_assert(KnownNativeCpuCall<0x801A98B0u>::kAvailable);

extern "C" __attribute__((noinline, used)) void synthetic_os_context_hle_probe(CpuContext* cpu) {
    if (!cpu) {
        return;
    }
    const auto savedR3 = cpu->gpr[3];
    InvokeDirectCpu<0x801A98B0u>(cpu);
    cpu->gpr[3] = savedR3;
}
