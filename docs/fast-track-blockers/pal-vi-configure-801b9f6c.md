# PAL fast-track blocker — VIConfigure (`0x801B9F6C`)

Real Switch evidence after PR #131:

```text
kind: DIRECT
target: 0x801b9f6c
pc: 0x800060a4
r1: 0x80399108
r2: 0x8038efa0
r3: 0x802457e4
r13: 0x8038cc00
stage: GUEST_POST_MAIN_ACTIVE
```

RMCP01 and pinned WiiCompiled map `0x801B9F6C` to `VIConfigure()`.

The pinned HLE validates the guest `GXRenderModeObj`, decodes TV format and geometry into VI pending state, then returns `r3 = 0`. Its host presenter call is not part of the headless Switch fast-track.

The Switch bridge keeps the pending TV format, render dimensions, VI origins and XFB dimensions. The synthetic probe uses the hardware-observed `r3 = 0x802457E4` shape.

Acceptance: the next hardware run must pass `0x801B9F6C` and expose the next blocker or named phase.
