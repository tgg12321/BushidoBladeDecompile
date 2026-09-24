# func_8005E098 — evidence (manual session 2026-09-23)

Started from nothing: no earlier ledger, and `pre-include-asm-body.c` was a placeholder
with no logic. The first full draft scored 59/289. The candidate reached 0 in the same session.

## What the function is
Draws the two-digit readout. Loop 1 draws a header sprite, or when arg0 < 0 a single
sprite that pokes D_8009B48E. Loop 2 splits arg1/arg0 into tens and units digits in
`s.d[]` and draws them, skipping a leading zero when arg0 < 0. Loop 3 draws two
semi-transparent TILEs plus two sprites each. The function then adds a DR_MODE and returns
the primitive-buffer bytes it used (0x304). Every loop runs once when arg0 < 0 (the
`func_8005E51C` wrapper passes -1).

## Levers, measured in order (sandbox --disable all --candidate)
- 59 → 45: a `base` pointer for loop 3 (its effect was later superseded by the table form).
- 45 → 11: **the digit stores sit in each arm**: `if (j) s.d[0] = s.d[1] = arg0; else ...
  = arg1;`. The target reloads both halfwords (`lhu 72/74(sp)`) right after storing them.
  That happens only when CSE cannot see the stored value at the join. Cross-jumping later
  merges the two arms' tails into the shared `move v0,…; sh; sh`.
  KILLED (all 45, CSE reuses the register): ternary into s32/s16 temp, if/else into a temp
  then shared stores, u16 d[], `(s16 *)` read casts, pd/ps pointer aliases (45–115), and
  index-var stores (48–115). The loop-store `for` gave 26.
- 11 → 7: **D_8009B398 is a table of 12-byte records** (`s32 D_8009B398[][3]`). Loop 3's
  `[2]`/`[3]` and the DR_MODE's `[0]` then derive from one hoisted register
  (`addiu s7,…; s7+12; s7-24`), which matches the target.
- 7 → 0: **D_8009B458 is a table of 8-byte records** (`s32 D_8009B458[][2]`: loop 1 `[i]`,
  loop 3 `[j*2+2]`/`[j*2+3]`). The `[j][0]/[j][1]` 3-D form also scores 0. Separate
  `D_8009B468`/`D_8009B470` arrays stay at 7.
- The final tidy (D_8009B400 as `[][2]` records, unused locals dropped) is still 0. The full
  build SHA1 == oracle.

## Layer-2 round 1: FAIL (the body was fine; the aggregate-merge prongs were not met)
The reviewer accepted every body construct. It failed the TU-local, partial table merge:
prong (c), because `extern s32 D_8009B3B0` was still in func_80060544 and the named_syms
rows had no alias suffix; prong (d), because the tables were declared TU-local and not in a
shared header; and prong (b), because the `j*2+2` magic stride stood in for record pairs.
Remedy applied: record types plus bounded tables `D_8009B398[4]`, `D_8009B400[10]` and
`D_8009B458[3][2]` in include/game.h, with evidence comments. func_80060544 now reads
`(s32 *)&D_8009B398[2]`. The named_syms rows at 0x8009B3A4/0x8009B3B0 carry
`alias of D_8009B398+N` and name every remaining asm referrer (round 2 FAILed an
earlier suffix that named only func_8005D814). 0x3A4 is referenced by D814 and F1C8;
0x3B0 by D814, E54C and F1C8. Loop 3 is indexed `[j + 1][0/1]`. The
sandbox still scores 0 (header block inlined, since the tree stays clean) and the full build
SHA1 == oracle.
