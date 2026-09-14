# PAL fast-track blocker — VIGetDTVStatus (`0x801BAD38`)

A real Switch hardware run after PR #129 crossed PAL `SCGetAspectRatio` (`0x801B1BE4`) and stopped at a new unsupported `DIRECT` dispatch:

```text
kind                  : DIRECT
target                : 0x801bad38
guest pc              : 0x800060a4
r1                    : 0x803990e8
r2                    : 0x8038efa0
r3                    : 0x00000001
r13                   : 0x8038cc00
fast-track stage      : GUEST_POST_MAIN_ACTIVE
```

RMCP01 / pinned WiiCompiled maps `0x801BAD38` to `VIGetDTVStatus()`.

At WiiCompiled pin `a135beb201042b20f390c6695ca6b26768820fb4`, this is a native override. The original Wii implementation would read VI MMIO at `0xCC00206E`; the pinned HLE deliberately skips that hardware access and returns `0`, documented as DTV not ready / disabled. No guest-memory, retrace, framebuffer, Aurora or renderer state is changed at this boundary.

The Switch fast-track therefore mirrors that exact guest-visible contract with `KnownNativeCpuCall<0x801BAD38>` setting `r3 = 0`. A Nintendo-data-free synthetic probe resolves the same direct target and seeds the hardware-observed `r3 = 1` input shape.

Acceptance criterion for the next hardware run: `0x801BAD38` must no longer be the durable unsupported dispatch; the next blocker or named post-main phase becomes the next implementation target.
