# PAL blocker: NANDOpenAsync (0x8019C918)

A real Switch fast-track run captured PAL `NANDOpenAsync` as the next `DIRECT` blocker while translated startup remained active.

Pinned WiiCompiled commit `a135beb201042b20f390c6695ca6b26768820fb4` maps `0x8019C918` to `NANDOpenAsync_HLE` in `runtime/src/hle/storage/nand_async.cpp`.

Pinned semantics are:

1. call synchronous `NANDOpen` with `(path, fileInfo, mode)`;
2. queue the guest completion callback as `(result, commandBlock)`;
3. return the same NAND result immediately.

The Switch bridge reuses the existing SD-backed `OpenSync` implementation and persistent host-fd table introduced for `NANDPrivateOpenAsync`, then queues and drains the callback through the existing scratch-`CpuContext` callback runtime.

As with the private-open bridge, draining before the HLE returns is currently a fast-track scheduling approximation. Pinned WiiCompiled normally drains NAND callbacks later through its IOS/alarm servicing path. Hardware remains authoritative for whether that ordering is sufficient.
