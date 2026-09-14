# PAL fast-track blocker — OSLockMutex (`0x801A7EE4`)

Real Switch hardware reached a new unsupported `DIRECT` dispatch after the Wii boot low-memory/MEM2 arena fix landed.

Observed durable blocker:

```text
kind                  : DIRECT
target                : 0x801a7ee4
guest pc              : 0x800060a4
r1                    : 0x803990d8
r2                    : 0x8038efa0
r3                    : 0x80346d00
r13                   : 0x8038cc00
fast-track stage      : GUEST_POST_MAIN_ACTIVE
action                : abort after durable blocker record
```

`0x801A7EE4` is PAL `OSLockMutex`. The pinned WiiCompiled runtime native-overrides this entry point in `runtime/src/hle/os/os_scheduler.cpp`; it is therefore intentionally absent from translated-code coverage and must be supplied by the Horizon native/HLE catalogue.

The Switch bridge now preserves the pinned uncontended and recursive semantics:

- disable/restore guest interrupt state through the existing shared interrupt HLE;
- use the guest `OSRunningContext` at `0x800000E4` as current thread;
- set/read mutex owner at `+0x08` and recursion count at `+0x0C`;
- link first acquisition into the thread held-mutex list (`+0x2F4/+0x2F8`) through the mutex links (`+0x10/+0x14`);
- provide the paired `OSUnlockMutex` (`0x801A7FC0`) no-waiter/no-priority-inheritance path so startup does not immediately stop at the matching native boundary.

Contended sleep/wakeup and priority inheritance are deliberately *not* approximated. If startup reaches those cases before the scheduler/fiber HLE is ported, the bridge emits a specific durable blocker (`OSLOCKMUTEX_CONTENDED`, `OSUNLOCKMUTEX_WAITERS`, or `OSUNLOCKMUTEX_INHERITED_PRIORITY`) and aborts.

Expected next hardware result: direct dispatch `0x801A7EE4` should no longer be the blocker. If the observed startup lock is uncontended, execution should continue into the next translated/native boundary.
