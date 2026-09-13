# PAL blocker: NANDCloseAsync (0x8019CAEC)

A real Switch fast-track run captured PAL `NANDCloseAsync` as the next `DIRECT` blocker while translated startup remained active.

Pinned WiiCompiled commit `a135beb201042b20f390c6695ca6b26768820fb4` maps `0x8019CAEC` to `NANDCloseAsync_HLE` in `runtime/src/hle/storage/nand_async.cpp`.

Pinned semantics are:

1. call synchronous `NANDClose(fileInfo)`;
2. queue the completion callback as `(result, commandBlock)`;
3. return that synchronous close result verbatim.

Unlike `NANDReadAsync`, this wrapper does not collapse a non-negative transfer result to zero; close already returns a NAND status code.

The Switch bridge reuses the persistent SD-backed NAND fd table created by `NANDOpenAsync`, closes/removes the host handle, marks `NANDFileInfo::openFlag` closed, queues the existing guest callback bridge, and preserves the current fast-track immediate callback drain timing.
