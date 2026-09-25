# func_80063E10 — hypotheses / receipts (manual lane, 2026-09-25)

Every no-semantic-purpose construct in candidate.c, with what was measured
without it. Scores are sandbox --disable all on the full candidate with only
that construct changed; bodies in rejected/.

## H1 — base staged through `prim` (staged-value-reused-variable, as func_8006295C FAKE 1)
`prim = (POLY_FT4 *)D_800A34EC;` derives mats/cm/sxy/zbuf/zn, then
`prim = (POLY_FT4 *)D_800A37D4;` gives prim its cursor job. prim has no prior
live value; the staged base is dead before prim's real use.
- fresh `u8 *base` local: 12 (rejected/fresh-base-local-12.c): single-block
  pseudo, local-alloc v0; the zn add can no longer fill the beqz delay slot
  and `li 10` moves into it (5 source-level hunks).
- reading D_800A34EC directly for each pointer (tmp/f3e10/q.c): 12.

## H2 — `(bit = 1 << i)` embedded in the slot test (bit not read again)
- `bit = 1 << i;` statement, then `D_800A3454[lane] & bit`: 41
  (rejected/bit-statement-before-test-41.c): address computed next to the
  load (life 2), not hoisted; recomputed in-loop.
- `bits = D_800A3454[lane]; bit = 1 << i; bits & bit`: 40.
- inline `D_800A3454[lane] & (1 << i)` / `(1 << i) & ...` / `(x >> i) & 1`:
  44 / 44 / 44 (rejected/inline-shift-test-44.c): srav/andi, 429 insns.
- `bit & D_800A3454[lane]` with bit statement: 43.
- `s32 *live = &D_800A3454[lane];` before the loop: 14 (computed before the
  guard; target has it after the guard = loop.c preheader) (rejected/live-pointer-before-loop-14.c).
- `live = &D_800A3454[lane]; bit = 1 << i; *live & bit` inside the loop: 9
  (hoisted, but `&arr[idx]` expands sll before la; target la first)
  (rejected/live-pointer-in-loop-9.c); `live = D_800A3454 + lane` same 9.
- opaque `one = 1` and `D_800A3454[lane] & (one << i)`: 35.
- `(bit = D_800A3454[lane]) & (1 << i)`: 43.

## H3 — tail walks the buffer again with `prim` (end = prim)
- fresh cursor `for (end = D_800A37D4; end < prim; end++)`: 36
  (rejected/fresh-tail-cursor-36.c): prim loses its tail refs, global.c
  seats sxy in s2 and prim in s3 (24 operand-only hunks).
- walking `*zbuf++` instead of zbuf[k]: 79.

## H4 — mats[i].t stores before `&mats[i]` operands (ordinary C, recorded)
`m = &mats[i]` at the top of the body: 4 (rejected/m-giv-before-rec-giv-4.c,
giv init order). A record pointer `rec = &D_800F0EC8[lane][i]`: 16.

## Not tried / open
- Permuter was not run (closed by hand in one session).
