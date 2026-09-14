# PAL `OSGetCurrentThread` — `0x801A98B0`

Tracking: #117

## Hardware evidence

After PR #123 allowed the real Switch run to cross PAL `OSLockMutex` (`0x801A7EE4`), the next durable unsupported direct-dispatch record was:

```text
kind    : DIRECT
target  : 0x801A98B0
pc      : 0x800060A4
r1      : 0x803990E8
r2      : 0x8038EFA0
r3      : 0x00000010
r13     : 0x8038CC00
stage   : GUEST_POST_MAIN_ACTIVE
```

## Mapping and pinned semantics

At pinned WiiCompiled commit `a135beb201042b20f390c6695ca6b26768820fb4`, `0x801A98B0` is registered as the native `OSGetCurrentThread` implementation.

The native HLE reads the guest OS running-context pointer from low memory and returns that value. On an invalid guest-memory read it returns `0`. It does not create a host thread, modify scheduler state, or synthesize a guest thread object.

This matches the SDK role of `OSGetCurrentThread` and is the value consumed by early mutex/thread bookkeeping.

## Switch bridge

PR #124 adds a `KnownNativeCpuCall<0x801A98B0>` bridge that mirrors the pinned return-value behavior and adds a Nintendo-data-free synthetic direct-dispatch probe.

The PR is merged. Hardware validation is still pending: the next real-Switch run must prove that `0x801A98B0` is crossed and reveal the next durable post-main boundary.

Current `main` immediately after the implementation merge:

```text
ac9ff53d0d54146aa9a2b187b533c25c2e2f852a
```
