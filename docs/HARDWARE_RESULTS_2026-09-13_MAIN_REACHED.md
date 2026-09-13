# Hardware result — PAL main reached (2026-09-13)

A real Nintendo Switch fast-track run reached PAL Mario Kart Wii `main()` at `0x8000B6B0`.

Observed durable liveness record:

```text
dispatch count        : 605
last target           : 0x8000b6b0
fast-track stage      : GUEST_MAIN_REACHED
PAL main              : 0x8000b6b0
main reached          : YES
```

This proves that the translated startup path from PAL `__start` through the early Wii OS/DVD/ESP/NAND bootstrap can reach the game entry point on real Switch hardware.

The fast-track diagnostic hook only records the milestone; it does not intentionally stop execution when `main()` is reached. Follow-up work therefore continues directly into post-main game/resource initialization under issue #117.

This evidence does **not** mean that a game frame has rendered. The GX FIFO bridge remains a deliberate sink and the real GX -> Switch renderer is still pending.
