#include "abi_bridge.h"

#if defined(MKW_SYNTHETIC_FAST_TRACK) && MKW_SYNTHETIC_FAST_TRACK

static_assert(KnownNativeCpuCall<0x8016B850u>::kAvailable);

// Nintendo-data-free compile/link coverage for PAL GXInit. The FIFO base and
// size are fabricated probe values; the Switch bridge must not depend on game
// assets or a renderer being present.
extern "C" __attribute__((used)) void synthetic_gx_init_hle_probe(CpuContext* ctx) {
    if (!ctx) {
        return;
    }

    ctx->gpr[3] = 0x80010000u;
    ctx->gpr[4] = 0x00001000u;
    InvokeDirectCpu<0x8016B850u>(ctx);
}

#endif
