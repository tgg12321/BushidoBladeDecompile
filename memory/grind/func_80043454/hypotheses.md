# func_80043454 — ruled out (manual session 2026-09-25)

- `while (count--)` post-decrement: GCC keeps the old value (`move v0,s; beqz v0`), target
  compares the decremented value against -1 -> `--count != -1`.
- default placement in the mode-1/2 inner switches (first / no break): no effect (130/115).
- `for (count--; count != -1; count--)`, `if (--c != -1) do {} while (--c != -1)`: same
  reorg inversion as while (115) — the inversion is the outer loop's continue label, not the
  inner loop form.
- outer do-while as a goto loop: loses the loop.c hoists out of the outermost loop (186-231).
- header value in a separate variable (v8a, 82): count seat fixed but header lands in a0.
- per-case counter `i = count` (v10a, 63) / header in `i` (v12a, 106): wrong seats.
- all three case loops as goto loops (v6b, 279): case 0 needs loop.c hoisting.
- kind via ternary / `kind = t - 0x26; if ((u32)kind >= 4) ...`: 115 / 119.
- align-up spellings `(u32)p + 4 - (p&3)`, `(u8*)p - (p&3) + 4`, `(s32)` casts: 7 / 4 / 7;
  `(u8*)p + (4 - (p&3))` and `(u32)p + (4 - (p&3))` both 0.
