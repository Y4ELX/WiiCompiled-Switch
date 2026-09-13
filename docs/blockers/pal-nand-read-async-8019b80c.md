# PAL blocker: NANDReadAsync (0x8019B80C)

A real Switch fast-track run captured PAL `NANDReadAsync` as the next `DIRECT` blocker while translated startup remained active after `NANDOpenAsync`.

Pinned WiiCompiled commit `a135beb201042b20f390c6695ca6b26768820fb4` maps `0x8019B80C` to `NANDReadAsync_HLE` in `runtime/src/hle/storage/nand_async.cpp`.

Pinned semantics are:

1. call synchronous `NANDRead(fileInfo, buffer, length)`;
2. queue the guest completion callback as `(rawResult, commandBlock)`, where a non-negative raw result is the transferred byte count;
3. return the negative error unchanged, otherwise return `NAND_RESULT_OK` (`0`).

The Switch bridge reuses the existing SD-backed NAND fd table created by `NANDOpenAsync` / `NANDPrivateOpenAsync`, reads directly into validated guest memory, queues the raw completion result and keeps the current fast-track callback drain on a scratch `CpuContext`.

A post-fix hardware run is required before marking this boundary hardware-crossed.
