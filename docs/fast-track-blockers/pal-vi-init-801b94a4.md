# PAL VIInit — 0x801B94A4

Tracking: #117

## Hardware evidence

A real Switch run after the GXInit bridge crossed `0x8016B850` and stopped on the next unsupported direct boundary:

```text
kind    : DIRECT
target  : 0x801B94A4
pc      : 0x800060A4
r1      : 0x80399128
r2      : 0x8038EFA0
r3      : 0x804293D0
r13     : 0x8038CC00
stage   : GUEST_POST_MAIN_ACTIVE
```

For PAL RMCP01, `0x801B94A4` is `VIInit`.

## Pinned WiiCompiled semantics

At WiiCompiled pin `a135beb201042b20f390c6695ca6b26768820fb4`, both PAL `VIInit` (`0x801B94A4`) and lower-level `__VIInit` (`0x801B9294`) are native-overridden through the same `SeedViStateForInit` helper.

The host runtime deliberately does not touch Wii VI MMIO at `0xCC0020xx`. On first initialization it publishes generic VI defaults into the guest-visible SDK globals: initialized/timing flags, NTSC-format default, 640x480 render/XFB dimensions, zero retrace count, null pre/post-retrace callbacks and null pending framebuffer. The call returns zero in `r3`.

The Switch bridge mirrors only those guest-visible semantics. It does not import Aurora, create a framebuffer, emulate retrace timing, or claim renderer support.

## Validation

A Nintendo-data-free synthetic probe resolves both direct targets and invokes them without game assets or renderer state. Hardware validation requires the next real-Switch run to cross `0x801B94A4` and expose the following durable blocker or phase.
