# PAL post-main MEM2 arena bootstrap — `OSInitAlloc` `0x801A0FC8`

## Hardware evidence

The bounded real-Switch post-main trace reached:

```text
606  0x80008EF0  System::RKSystem::main
607  0x80009194  System::RKSystem::initialize
608  0x80242504  EGG::BaseSystem::initMemory
609  0x801A0FC8  OSInitAlloc  r3=0x8039B180
610  0x801A0FC8  OSInitAlloc  r3=0xFFFFFFFF
```

RMCP01 `EGG::BaseSystem::initMemory` obtains MEM1 and MEM2 arena bounds and
calls `OSInitAlloc` once for each arena. The first call is therefore the MEM1
allocator initialization and the second call is the MEM2 allocator
initialization.

The RMCP01 Revolution OS arena implementation initializes
`__OSMEM2ArenaLo` to `0xFFFFFFFF` and `__OSMEM2ArenaHi` to zero. Seeing
`r3=0xFFFFFFFF` on the second `OSInitAlloc` call proves that MEM2 arena low
was never published by translated `OSInit`.

## Root cause

Pinned WiiCompiled commit
`a135beb201042b20f390c6695ca6b26768820fb4` runs this startup order:

```text
Memory::Init
-> SystemBridge::SeedLowMemDefaults
-> InitializeDataSections
-> translated startup
```

The Switch bootstrap previously performed `Memory::Init` and generated data
initialization but omitted the pinned `SeedLowMemDefaults` contract.

That contract seeds the loader/IOS low-memory words consumed by translated
`OSInit`, including:

- MEM1 physical/simulated sizes and arena bounds;
- MEM2 physical/simulated size;
- `0x80003124` MEM2 arena low;
- `0x80003128` MEM2 arena high;
- MEM2 addressable end and IPC/IOS reservation bounds.

The pin maps 128 MiB of MEM2. With its 128 KiB IOS reservation, 128 KiB IPC
reservation, and 2 MiB runtime FST reservation, the relevant resulting values
are:

```text
MEM2 size       = 0x08000000
MEM2 arena lo   = 0x90000800
MEM2 arena hi   = 0x97DC0000
IPC buffer lo   = 0x97FC0000
IPC buffer hi   = 0x97FE0000
IOS reserved hi = 0x98000000
```

These are runtime/platform bootstrap values, not Nintendo game data.

## Fix

Restore the memory/arena subset of pinned `SystemBridge::SeedLowMemDefaults`
before generated data-section initialization. Do not patch `OSInitAlloc` and
do not replace translated `OSInit`: once low memory has the expected boot
contract, the original translated OS code remains responsible for publishing
and consuming the arena state.

A Nintendo-data-free synthetic probe validates both the derived layout and the
low-memory writes.

Tracking: #117
