#include "abi_bridge.h"

#if defined(MKW_SYNTHETIC_FAST_TRACK) && MKW_SYNTHETIC_FAST_TRACK

static_assert(KnownNativeCpuCall<0x801B9F6Cu>::kAvailable);

// Nintendo-data-free compile/link coverage for PAL VIConfigure.
// Seed r3 with the real hardware-observed GXRenderModeObj guest pointer shape.
extern "C" __attribute__((used)) void synthetic_vi_configure_hle_probe(CpuContext* ctx) {
    if (!ctx) {
        return;
    }

    ctx->gpr[3] = 0x802457E4u;
    InvokeDirectCpu<0x801B9F6Cu>(ctx);
}

#endif
