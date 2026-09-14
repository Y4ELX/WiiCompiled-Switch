# PAL fast-track blocker — SCGetAspectRatio (`0x801B1BE4`)

A real Switch hardware run after PR #128 crossed PAL `SCGetEuRgb60Mode` (`0x801B1CAC`) and stopped at a new unsupported `DIRECT` dispatch:

```text
kind                  : DIRECT
target                : 0x801b1be4
guest pc              : 0x800060a4
r1                    : 0x803990e8
r2                    : 0x8038efa0
r3                    : 0x00000001
r13                   : 0x8038cc00
fast-track stage      : GUEST_POST_MAIN_ACTIVE
```

RMCP01 / pinned WiiCompiled maps `0x801B1BE4` to `SCGetAspectRatio()`.

At WiiCompiled pin `a135beb201042b20f390c6695ca6b26768820fb4`, the native override returns `RuntimeConfigFile::WidescreenEnabled(true) ? 1u : 0u`. The fallback argument is `true`, so in the absence of an explicit runtime override the guest-visible result is `1` (16:9). The current Switch fast-track has no widescreen runtime-config surface, so it mirrors that pinned default path rather than inventing a separate setting.

The bridge therefore sets `r3 = 1` and leaves guest memory, NAND/IOS, VI state and the headless renderer untouched. A Nintendo-data-free synthetic probe resolves the same direct target with the hardware-observed `r3 = 1` input shape.

Acceptance criterion for the next hardware run: `0x801B1BE4` must no longer be the durable unsupported dispatch; the next blocker or named phase becomes the next implementation target.
