# Hardware result: PAL post-main application entry reached

Date: 2026-09-14
Tracking: #117

## Evidence

A real Switch run advanced beyond PAL `main()` at `0x8000B6B0` and recorded dispatch 606 at `0x80008EF0`, mapped to `System::RKSystem::main(int, char**)`, with stage `GUEST_POST_MAIN_ACTIVE`.

Because the normal heartbeat is time-gated, that single record did not prove that `0x80008EF0` was the final translated target executed before the observed stall. A bounded ordered post-main trace was therefore added.

## Bounded post-main trace

The diagnostics write:

```text
sdmc:/switch/WiiCompiled-Switch/fast-track-post-main-trace.txt
```

The trace has a fixed capacity of 64 entries. The first 48 post-main dispatches are captured densely; later capacity is reserved for known phase targets. Each record contains the global dispatch count, post-main index, target, guest PC, `r1`, `r2`, `r3`, `r13`, stage, and a phase label when known.

Writes are batched every eight dense dispatches and forced at known phase boundaries, avoiding an `fsync` on every translated dispatch.

Named phase milestones include:

```text
0x80008EF0  System::RKSystem::main
0x80008FB4  EGG::BaseSystem::initialize
0x80009194  System::RKSystem::initialize
0x8000951C  System::RKSystem::run
0x80243D18  EGG::Video::initialize
0x80243D6C  EGG::Video::configure
```

`fast-track-post-main-dispatch.txt` remains the one-shot record of the first dispatch after PAL `main()`.

## Hardware progression after the first post-main trace

The ordered trace proved that execution progressed beyond `RKSystem::main` into `RKSystem::initialize` and `EGG::BaseSystem::initMemory`. The first attributable post-main defect was the second PAL `OSInitAlloc` call at `0x801A0FC8`, which received `r3 = 0xFFFFFFFF` instead of a valid MEM2 arena start.

The pinned WiiCompiled bootstrap seeds Wii low-memory defaults before translated data-section initialization. The Switch fast-track had omitted that seed. PR #122 restored the pinned boot low-memory/MEM2 arena contract without bypassing translated `OSInitAlloc`. The next hardware run crossed that point, validating the fix.

The following hardware blocker was `OSLockMutex` at `0x801A7EE4`. PR #123 added the pinned-style uncontended/recursive mutex bridge while keeping real contention and priority-inheritance paths explicit. The next hardware run crossed that address.

The next blocker was `OSGetCurrentThread` at `0x801A98B0`. PR #124 mirrored the pinned native function's guest running-context return value. The subsequent real-Switch run crossed that address too, validating the bridge far enough to reach graphics FIFO initialization.

## Latest hardware blocker: GXInit

The latest durable blocker is:

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

For PAL RMCP01, doldecomp places this target in `gxInit.o`, and `EGG::GraphicsFifo` calls `GXInit` after allocating and aligning the FIFO buffer. At pinned WiiCompiled commit `a135beb201042b20f390c6695ca6b26768820fb4`, `0x8016B850` is explicitly native-overridden as `GX__Init_8016b850`.

PR #126 mirrors the pin's guest-visible GX initialization contract: GXData publication/defaults, FIFO/PE queues, current-thread bookkeeping, CP/PE interrupt handlers and the pinned FIFO object return value. It intentionally does not import Aurora or claim renderer initialization. A Nintendo-data-free synthetic probe resolves the same direct-dispatch trait with fabricated FIFO arguments.

## Current next hardware run

After PR #126 is merged, rebuild the local fast-track NRO from `main`, run it on hardware, and collect at minimum:

```text
fast-track-dispatch-blocker.txt
fast-track-post-main-trace.txt
```

Also keep these when present:

```text
fast-track-post-main-last-dispatch.txt
fast-track-post-main-dispatch.txt
fast-track-heartbeat.txt
fast-track-exception.txt
```

The immediate acceptance criterion is that `0x8016B850` is no longer reported as an unsupported direct dispatch. The next durable blocker, or a later named post-main phase, defines the next implementation step.

A black screen remains expected. The fast-track GX FIFO bridge is still a deliberate sink; this GXInit bridge is boot-state compatibility, not the M3 renderer.
