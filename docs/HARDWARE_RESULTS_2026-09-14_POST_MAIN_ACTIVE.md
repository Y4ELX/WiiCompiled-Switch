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

`GXInit` at `0x8016B850` was then reached. PR #126 mirrored the pin's guest-visible GXData/FIFO/PE initialization while deliberately leaving the Switch renderer headless. The subsequent hardware run crossed `GXInit`, validating that boot-state bridge and exposing `VIInit`.

`VIInit` at `0x801B94A4` was then reached. PR #127 mirrored pinned WiiCompiled's VI first-initialization contract without Wii MMIO, Aurora, a fabricated framebuffer or synthetic retraces. The subsequent real-Switch run crossed `VIInit`, validating that boundary and exposing the first post-main SYSCONF getter.

`SCGetEuRgb60Mode` at `0x801B1CAC` was then reached. PR #128 mirrored the pin's direct PAL60/RGB60 result (`r3 = 1`) without NAND/IOS, VI or renderer side effects. The subsequent real-Switch run crossed that boundary too, validating the bridge and exposing the next SYSCONF getter.

## Latest hardware blocker: SCGetAspectRatio

The latest durable blocker is:

```text
kind    : DIRECT
target  : 0x801B1BE4
pc      : 0x800060A4
r1      : 0x803990E8
r2      : 0x8038EFA0
r3      : 0x00000001
r13     : 0x8038CC00
stage   : GUEST_POST_MAIN_ACTIVE
```

For PAL RMCP01, `0x801B1BE4` is `SCGetAspectRatio()`. At pinned WiiCompiled commit `a135beb201042b20f390c6695ca6b26768820fb4`, the native override returns `RuntimeConfigFile::WidescreenEnabled(true) ? 1u : 0u`. The fallback is therefore widescreen/16:9 unless an explicit runtime setting disables it.

The current Switch fast-track has no equivalent widescreen runtime-config surface, so the bridge mirrors the pinned default path with `r3 = 1`. It does not modify guest memory, NAND/IOS state, VI state or the headless renderer. A Nintendo-data-free synthetic probe resolves the same direct target with the hardware-observed `r3 = 1` input shape.

## Current next hardware run

After the `SCGetAspectRatio` bridge is merged, rebuild the local fast-track NRO from `main`, run it on hardware, and collect at minimum:

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

The immediate acceptance criterion is that `0x801B1BE4` is no longer reported as an unsupported direct dispatch. The next durable blocker, or a later named post-main phase, defines the next implementation step.

A black screen remains expected. GX and VI bridges here are boot-state compatibility only; the fast-track GX FIFO remains a deliberate sink until the M3 renderer exists.
