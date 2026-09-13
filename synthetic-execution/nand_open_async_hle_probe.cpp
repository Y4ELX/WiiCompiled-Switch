#include "abi_bridge.h"

#if defined(MKW_SYNTHETIC_FAST_TRACK) && MKW_SYNTHETIC_FAST_TRACK

// Nintendo-data-free compile/link coverage for PAL NANDOpenAsync
// (0x8019C918). Null guest pointers deliberately exercise only the safe
// invalid-argument path if executed while still proving exact native dispatch.
extern "C" __attribute__((used)) void synthetic_nand_open_async_hle_probe(CpuContext* ctx) {
    if (!ctx) {
        return;
    }

    ctx->gpr[3] = 0u;
    ctx->gpr[4] = 0u;
    ctx->gpr[5] = 1u;
    ctx->gpr[6] = 0u;
    ctx->gpr[7] = 0u;
    InvokeDirectCpu<0x8019C918u>(ctx);
}

#endif
