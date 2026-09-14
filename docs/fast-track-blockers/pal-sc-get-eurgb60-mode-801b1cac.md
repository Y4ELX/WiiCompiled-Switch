# PAL fast-track blocker — SCGetEuRgb60Mode (`0x801B1CAC`)

A real Switch hardware run after PR #127 crossed PAL `VIInit` (`0x801B94A4`) and stopped at a new unsupported `DIRECT` dispatch:

```text
kind                  : DIRECT
target                : 0x801b1cac
guest pc              : 0x800060a4
r1                    : 0x803990e8
r2                    : 0x8038efa0
r3                    : 0x00000000
r13                   : 0x8038cc00
fast-track stage      : GUEST_POST_MAIN_ACTIVE
```

RMCP01 / pinned WiiCompiled maps `0x801B1CAC` to `SCGetEuRgb60Mode()`.

At WiiCompiled pin `a135beb201042b20f390c6695ca6b26768820fb4`, the function is a native override that returns `1` directly. The pin documents the result as PAL60/RGB60 and intentionally chooses that mode for PAL builds instead of PAL50. There are no guest-memory, NAND, IOS or renderer side effects at this boundary.

The Switch fast-track therefore mirrors the pin exactly with a `KnownNativeCpuCall<0x801B1CAC>` specialization that sets `r3 = 1`. A Nintendo-data-free synthetic probe resolves the same direct target and seeds `r3 = 0`, matching the observed hardware blocker shape.

Acceptance criterion for the next hardware run: `0x801B1CAC` must no longer be the durable unsupported dispatch; the next blocker or named phase becomes the next implementation target.
