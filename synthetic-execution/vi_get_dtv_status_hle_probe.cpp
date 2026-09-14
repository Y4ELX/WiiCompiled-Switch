#include "abi_bridge.h"

#if defined(MKW_SYNTHETIC_FAST_TRACK) && MKW_SYNTHETIC_FAST_TRACK

static_assert(KnownNativeCpuCall<0x801BAD38u>::kAvailable);

// Nintendo-data-free compile/link coverage for PAL VIGetDTVStatus.
// Seed r3 with the real hardware blocker value; the pinned native boundary
// must replace it with 0 (DTV not ready / disabled).
extern "C" __attribute__((used)) void synthetic_vi_get_dtv_status_hle_probe(CpuContext* ctx) {
    if (!ctx) {
        return;
    }

    ctx->gpr[3] = 1u;
    InvokeDirectCpu<0x801BAD38u>(ctx);
}

#endif
