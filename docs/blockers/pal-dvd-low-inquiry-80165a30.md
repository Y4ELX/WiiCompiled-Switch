# PAL DVDLowInquiry (`0x80165A30`)

Real Switch fast-track hardware reached a `DIRECT` unsupported-dispatch blocker at PAL `0x80165A30` while translated startup remained active (`TRANSLATED_EXEC_ENTER`). The captured guest command-block pointer was `r3 = 0x803470A0`.

Pinned WiiCompiled maps this entry point to `DVDLowInquiry`. Its host HLE acknowledges the drive as present, writes `DVD_STATE_END` (`0`) to the command block state field at `+0x0C`, completes the shared DVD cancel/reset bookkeeping, and returns `1` to report that the low-level request was successfully issued. The callback argument is not invoked by this override.

The Switch fast-track mirrors those guest-visible semantics and reuses the existing DVD cancel-state helper rather than duplicating the queue/global initialization.
