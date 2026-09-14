# Hardware result: PAL post-main application entry reached

Date: 2026-09-14
Tracking: #117

## Evidence

A real Switch run advanced beyond PAL `main()` at `0x8000B6B0` and recorded dispatch 606 at `0x80008EF0`, mapped to `System::RKSystem::main(int, char**)`, with stage `GUEST_POST_MAIN_ACTIVE`.

Because the normal heartbeat is time-gated, that single record does not prove that `0x80008EF0` was the final translated target executed before the observed stall.

## Bounded post-main trace

The diagnostics now write:

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

A synthetic formatter probe under `MKW_SYNTHETIC_FAST_TRACK` uses fabricated addresses and register values so the public build path can compile the trace logic without game-derived input.

## Next hardware run

Rebuild the local fast-track NRO from `main`, run it on hardware, and collect:

```text
fast-track-post-main-trace.txt
fast-track-post-main-dispatch.txt
fast-track-heartbeat.txt
fast-track-dispatch-blocker.txt
fast-track-exception.txt
```

Use the ordered trace to identify the furthest proven initialization phase before changing guest semantics. A black screen is still expected while the fast-track GX path remains headless.
