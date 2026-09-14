# PAL GXInit blocker — 0x8016B850

Tracking: #117
Date: 2026-09-14

## Hardware evidence

After PR #124, a real Switch run no longer stopped at `OSGetCurrentThread` (`0x801A98B0`). The next durable unsupported direct dispatch was:

```text
kind    : DIRECT
target  : 0x8016B850
pc      : 0x800060A4
r1      : 0x80399128
r2      : 0x8038EFA0
r3      : 0x803A9320
r13     : 0x8038CC00
stage   : GUEST_POST_MAIN_ACTIVE
```

This hardware progression validates the merged `OSGetCurrentThread` bridge far enough to reach the graphics FIFO initialization path.

## Mapping

For PAL RMCP01, doldecomp places the target inside `gxInit.o`; `EGG::GraphicsFifo` calls `GXInit` after allocating and aligning its FIFO buffer. At pinned WiiCompiled commit `a135beb201042b20f390c6695ca6b26768820fb4`, address `0x8016B850` is explicitly native-overridden as `GX__Init_8016b850`.

## Switch decision

PR #126 mirrors the pinned wrapper's guest-visible initialization only:

- publish/initialize the guest GXData block and pointer;
- publish GX FIFO/PE thread queues and current-thread bookkeeping;
- install the CP/PE interrupt handler addresses and clear their guest mask bits;
- initialize the pinned GXData shadow-register defaults;
- return the pinned guest FIFO object address.

The Switch fast-track intentionally does **not** import Aurora or claim renderer initialization. `GX_HLE_FIFO_Write*` remains a sink, so a black screen is still expected and first-frame validation remains an M3 task.

A Nintendo-data-free synthetic probe uses fabricated FIFO arguments and resolves the same direct-dispatch trait without game assets.

## Acceptance criterion

The next hardware run must no longer report `0x8016B850` as an unsupported direct dispatch. The next durable blocker, or a later named post-main phase, becomes the next implementation target.
