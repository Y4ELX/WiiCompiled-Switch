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

The following hardware blocker was:

```text
target  : 0x801A7EE4
symbol  : OSLockMutex
r3      : 0x80346D00
stage   : GUEST_POST_MAIN_ACTIVE
```

Pinned WiiCompiled native-overrides `OSLockMutex`/`OSUnlockMutex`. PR #123 added the Horizon-side mutex bridge for uncontended acquisition, recursive acquisition, paired unlock, guest owner/count bookkeeping and held-mutex list maintenance, while keeping real contention/priority-inheritance paths explicit rather than fabricating scheduler behavior. The next hardware run crossed `0x801A7EE4`, validating the startup path.

The latest hardware blocker is now:

```text
target  : 0x801A98B0
symbol  : OSGetCurrentThread
pc      : 0x800060A4
r1      : 0x803990E8
r3      : 0x00000010
r13     : 0x8038CC00
stage   : GUEST_POST_MAIN_ACTIVE
```

At pinned WiiCompiled commit `a135beb201042b20f390c6695ca6b26768820fb4`, `0x801A98B0` is registered as a native `OSGetCurrentThread` function that returns the guest running-context pointer from low memory. PR #124 mirrors that exact return-value behavior and adds Nintendo-data-free synthetic coverage.

PR #124 is merged. The corresponding bridge still requires the next real-Switch run for hardware validation. Current `main` after that merge is:

```text
ac9ff53d0d54146aa9a2b187b533c25c2e2f852a
```

## Current next hardware run

Rebuild the local fast-track NRO from current `main`, run it on hardware, and collect at minimum:

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

The immediate acceptance criterion is that `0x801A98B0` is no longer reported as an unsupported direct dispatch. The next durable blocker will then define the next implementation step.

A black screen remains expected while the fast-track GX path is headless and the GX FIFO bridge remains a sink.
