#include "abi_bridge.h"

#if defined(MKW_SYNTHETIC_FAST_TRACK) && MKW_SYNTHETIC_FAST_TRACK
extern "C" __attribute__((used)) void synthetic_dvd_inquiry_hle_probe(CpuContext* ctx) {
    if (!ctx) {
        return;
    }

    ctx->gpr[3] = 0x803470A0u;
    ctx->gpr[4] = 0u;
    InvokeDirectCpu<0x80165A30u>(ctx);
}
#endif
