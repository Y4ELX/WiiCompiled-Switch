# PAL blocker: ESP_CloseLib (0x80167224)

Hardware fast-track reached this DIRECT boundary while translated startup remained active.

Pinned WiiCompiled commit `a135beb201042b20f390c6695ca6b26768820fb4` maps this address to `ESP_CloseLib`. Because the host runtime never opened Wii `/dev/es`, the pinned native override performs no close operation and returns `0` immediately.

The Switch HLE mirrors that exact leaf behavior: no host handle close, no guest-memory writes, no callback side effects, and `r3 = 0` on return.
