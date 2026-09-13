#include "abi_bridge.h"

#if defined(MKW_SYNTHETIC_FAST_TRACK) && MKW_SYNTHETIC_FAST_TRACK

// Nintendo-data-free compile coverage for PAL DVDInit (0x8015EA1C). The probe
// carries the same r3 shape observed on hardware (0x80000000). No synthetic FST
// is fabricated: the HLE must remain safe when the runtime has no published DVD
// filesystem yet, while still resolving through the native dispatch catalogue.
extern "C" __attribute__((used)) void synthetic_dvd_init_hle_probe(CpuContext* ctx) {
    if (!ctx) {
        return;
    }

    ctx->gpr[3] = 0x80000000u;
    InvokeDirectCpu<0x8015EA1Cu>(ctx);
}

#endif
