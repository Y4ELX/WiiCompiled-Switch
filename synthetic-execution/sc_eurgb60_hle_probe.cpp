#include "abi_bridge.h"

#if defined(MKW_SYNTHETIC_FAST_TRACK) && MKW_SYNTHETIC_FAST_TRACK

static_assert(KnownNativeCpuCall<0x801B1CACu>::kAvailable);

// Nintendo-data-free compile/link coverage for PAL SCGetEuRgb60Mode.
// Seed r3 with the real hardware blocker value; the pinned native boundary
// must replace it with 1 (PAL60/RGB60).
extern "C" __attribute__((used)) void synthetic_sc_eurgb60_hle_probe(CpuContext* ctx) {
    if (!ctx) {
        return;
    }

    ctx->gpr[3] = 0u;
    InvokeDirectCpu<0x801B1CACu>(ctx);
}

#endif
