#include "abi_bridge.h"

#if defined(MKW_SYNTHETIC_FAST_TRACK) && MKW_SYNTHETIC_FAST_TRACK

static_assert(KnownNativeCpuCall<0x801B1BE4u>::kAvailable);

// Nintendo-data-free compile/link coverage for PAL SCGetAspectRatio.
// Seed r3 with the real hardware blocker value; the pinned default path must
// leave the guest-visible return value at 1 (16:9).
extern "C" __attribute__((used)) void synthetic_sc_aspect_ratio_hle_probe(CpuContext* ctx) {
    if (!ctx) {
        return;
    }

    ctx->gpr[3] = 1u;
    InvokeDirectCpu<0x801B1BE4u>(ctx);
}

#endif
