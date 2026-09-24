# func_8003E6D8 — ruled out (manual session 2026-09-23)

Measured with ra_solver priorities (bits vs a3) and sandbox; all stay at 9
unless noted:
- declaration order of bits / a3 (local-alloc/global priority is refs/livelen)
- a3 as u8 / s8 (flips the pair but spills x/z to $s3/$s4: 40, 300 insns);
  u16 / s16 / u32 (no flip)
- bits as u32 with (s32) or & 0x80000000 tests; `bits *= 2`; `bits = *mask;
  mask++`; `if (bits != 0) { ... }` wrapper
- mask++ in the outer for-increment / nested-if outer loop (21, 294 insns)
- `mask++; bits = mask[-1]` (4: emits lw -4 after the addiu)
- `j = 0` hoisted above the bits load (2: bits livelen 78->77 flips the pair
  but the move lands before the lw) -- rejected/j0-before-load-2.c
- separate `word` pseudo for the unshifted value (34)
- flag-update condition respelled: else-if chains / nested / ternary /
  multi-arm (E1, G3, G8, P1-P4) -- multi-arm forms flip the pair (livelen +6)
  but change the block layout (10-13) -- rejected/multiarm-flag-10.c
- do-while spellings (vidx++ placement, `out++` split, `(t0 & 0x8000) == 0`,
  `t0 >> 15`, list via `D_800A3820 += 4`, 13*8 multiply, `(row << 5) + col`)
- goto-based do-while (vidx takes $t1)
