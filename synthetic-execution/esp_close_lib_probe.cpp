#include "abi_bridge.h"

#if defined(MKW_SYNTHETIC_FAST_TRACK) && MKW_SYNTHETIC_FAST_TRACK
extern "C" __attribute__((used)) void synthetic_esp_close_lib_probe(CpuContext* ctx) {
    if (!ctx) {
        return;
    }
    ctx->gpr[3] = 1u;
    InvokeDirectCpu<0x80167224u>(ctx);
}
#endif
