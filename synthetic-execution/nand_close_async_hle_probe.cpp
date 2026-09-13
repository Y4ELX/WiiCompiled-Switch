#include "abi_bridge.h"

#if defined(MKW_SYNTHETIC_FAST_TRACK) && MKW_SYNTHETIC_FAST_TRACK

// Nintendo-data-free compile/link coverage for PAL NANDCloseAsync
// (0x8019CAEC). A null NANDFileInfo exercises the safe invalid-argument path
// if executed while proving this exact native dispatch boundary is retained.
extern "C" __attribute__((used)) void synthetic_nand_close_async_hle_probe(CpuContext* ctx) {
    if (!ctx) {
        return;
    }

    ctx->gpr[3] = 0u;
    ctx->gpr[4] = 0u;
    ctx->gpr[5] = 0u;
    InvokeDirectCpu<0x8019CAECu>(ctx);
}

#endif
