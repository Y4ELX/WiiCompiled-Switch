# PAL blocker: ESP_InitLib (0x801671D0)

Hardware fast-track captured:

```text
kind                  : DIRECT
target                : 0x801671d0
guest pc              : 0x800060a4
r1                    : 0x80399020
r2                    : 0x8038efa0
r3                    : 0x00000000
r13                   : 0x8038cc00
fast-track stage      : TRANSLATED_EXEC_ENTER
```

Pinned WiiCompiled commit `a135beb201042b20f390c6695ca6b26768820fb4` maps this PAL address to `ESP_InitLib`. Its native override does not open Wii `/dev/es`; it returns `0` immediately so early title-service initialization can continue.

Switch behavior mirrors that leaf HLE exactly by setting guest `r3 = 0`, with no guest-memory writes, no IOS handle, and no callback side effects.

A Nintendo-data-free synthetic probe dispatches `0x801671D0` through the translated ABI seam and starts from a non-zero `r3` value so the build retains and compiles the return-value override.
