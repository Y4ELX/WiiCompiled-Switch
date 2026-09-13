# PAL fast-track blocker — SCCheckStatus (`0x801B0220`)

Real Switch hardware reached a new unsupported `DIRECT` dispatch at PAL `0x801B0220` while the translated guest remained active in `TRANSLATED_EXEC_ENTER`.

Observed state:

```text
kind                  : DIRECT
target                : 0x801b0220
guest pc              : 0x800060a4
r1                    : 0x80399168
r2                    : 0x8038efa0
r3                    : 0xfffffff4
r13                   : 0x8038cc00
fast-track stage      : TRANSLATED_EXEC_ENTER
```

Pinned WiiCompiled `a135beb201042b20f390c6695ca6b26768820fb4` maps this address to `SCCheckStatus` and native-overrides it. The SDK normally polls this function while asynchronous SYSCONF loading is outstanding. The pinned host runtime does not emulate that Wii NAND/IOS completion path here, so its HLE returns `0` (`SC_STATUS_OK`) immediately to avoid an infinite `OSInit` busy loop.

The Switch port mirrors the same guest-visible result by setting `r3 = 0` and performing no Wii IOS access. The incoming `r3 = 0xFFFFFFF4` (`-12`) is treated only as caller state left by the preceding NAND operation; `SCCheckStatus` itself must overwrite it with success.

Nintendo-data-free synthetic coverage compiles the exact `InvokeDirectCpu<0x801B0220>` boundary with the observed `-12` sentinel.
