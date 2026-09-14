#include "abi_bridge.h"

#if defined(MKW_SYNTHETIC_FAST_TRACK) && MKW_SYNTHETIC_FAST_TRACK

static_assert(KnownNativeCpuCall<0x801BAB2Cu>::kAvailable);

// Nintendo-data-free compile/link coverage for PAL VISetBlack.
// Seed r3 with the real hardware blocker value; the pinned native boundary
// records the pending black request and returns 0 without fabricating retraces.
extern "C" __attribute__((used)) void synthetic_vi_set_black_hle_probe(CpuContext* ctx) {
    if (!ctx) {
        return;
    }

    ctx->gpr[3] = 1u;
    InvokeDirectCpu<0x801BAB2Cu>(ctx);
}

#endif
