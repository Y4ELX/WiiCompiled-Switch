#include "abi_bridge.h"

#if defined(MKW_SYNTHETIC_FAST_TRACK) && MKW_SYNTHETIC_FAST_TRACK
extern "C" __attribute__((used)) void synthetic_esp_init_lib_probe(CpuContext* ctx) {
    if (!ctx) {
        return;
    }

    ctx->gpr[3] = 0xFFFFFFFFu;
    InvokeDirectCpu<0x801671D0u>(ctx);
}
#endif
