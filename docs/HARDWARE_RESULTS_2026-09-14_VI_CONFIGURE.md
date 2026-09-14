# Hardware result: VIConfigure reached

Date: 2026-09-14
Tracking: #117

A real Switch run built from main after PR #131 crossed `VISetBlack` (`0x801BAB2C`) and stopped at PAL `VIConfigure` (`0x801B9F6C`).

```text
kind   : DIRECT
target : 0x801B9F6C
pc     : 0x800060A4
r1     : 0x80399108
r2     : 0x8038EFA0
r3     : 0x802457E4
r13    : 0x8038CC00
stage  : GUEST_POST_MAIN_ACTIVE
```

Pinned WiiCompiled validates the guest `GXRenderModeObj`, decodes TV format and geometry into pending VI state, and returns `r3 = 0`. Its host presenter call is intentionally not reproduced by the headless Switch fast-track.

PR #132 mirrors the pending VI bookkeeping and adds Nintendo-data-free synthetic coverage. The next hardware acceptance criterion is that `0x801B9F6C` is no longer the durable blocker.
