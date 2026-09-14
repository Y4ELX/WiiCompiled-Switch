# PAL fast-track blocker — VISetBlack (`0x801BAB2C`)

A real Switch hardware run after PR #130 crossed PAL `VIGetDTVStatus` (`0x801BAD38`) and stopped at a new unsupported `DIRECT` dispatch:

```text
kind                  : DIRECT
target                : 0x801bab2c
guest pc              : 0x800060a4
r1                    : 0x80399108
r2                    : 0x8038efa0
r3                    : 0x00000001
r13                   : 0x8038cc00
fast-track stage      : GUEST_POST_MAIN_ACTIVE
```

RMCP01 / pinned WiiCompiled maps `0x801BAB2C` to `VISetBlack()`.

At WiiCompiled pin `a135beb201042b20f390c6695ca6b26768820fb4`, the native HLE interprets `r3 != 0` as a black-screen request, stores it in the VI **pending** state, and returns `0`. The pending value is not made active immediately: `VIFlush` arms it and the next retrace commits it.

The Switch fast-track mirrors only that guest-visible contract. It preserves the requested pending-black boolean in the local VI bridge, returns `r3 = 0`, and deliberately does not fabricate `VIFlush`, a retrace, a framebuffer or a presenter. A Nintendo-data-free synthetic probe resolves the same direct target with the hardware-observed `r3 = 1` input shape.

Acceptance criterion for the next hardware run: `0x801BAB2C` must no longer be the durable unsupported dispatch. The next blocker or named post-main phase becomes the next implementation target.
