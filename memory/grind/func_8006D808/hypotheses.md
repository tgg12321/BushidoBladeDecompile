# func_8006D808 hypotheses

## 2026-09-24/25 manual session (closed 202 -> 0)

Codex s1: raw m2c + descriptor aggregate, 200/202 (superseded).

- CONFIRMED: family model — EnvA 0x2C descriptor (func_8007352C reads it only
  through +0x2B) and a separate 8-aligned s16 digit array at sp+0x48, digits by
  the func_8005E098 idiom. The three leading loops share one counter; the inner
  digit loop has its own. `s16 n` (its per-use sign-extensions reduce to the
  target's three copies s1/s5/s7). 124 -> 45.
- CONFIRMED: colour chain `col_r = col_g = col_b = 0xA0` (stores 0x43..0x41);
  loop A carries the value through s.header. 45 -> 34.
- CONFIRMED: per-player 4-byte records read as `D_800A3524 + (k << 2) + off`
  (the base register choice in both loop B and the three switch arms; `k * 4`
  and every other index spelling = 2..9 under the correct frame).

### Frame (8 bytes short: vars 64 vs target 72) — lever exhaustion

The target's sp+0x4C..0x57 is never touched; the only spill (0x58) is the
loop.c-hoisted `0 < n` flag. Instrument: instrumented cc1 BB2_FRAME_DEBUG
(tmp/e6d8/frame.sh; `vars=` and FRAMEDBG slot list) + combine `(use (reg N))`
orphan count. All below measured vars=64 / 0 orphans unless noted:
- honest producer 1 (folded guard): explicit `if (n > 0)` / `if (n != 0)` /
  `if (0 < n)` guards on loop B and the inner loop; `i = 0; if (i < n) do ...
  while (++i < n)` and while-forms on loop B (g1/g3/g4), inner (l5), loop A
  (s1/s4), outer (s2).
- honest producer 2 (HImode orphan): idx spellings x1-x6 (`idx = idx - 2`,
  `<= 21`, `> 11`, separate `c` local, `(u32)(c - 12) < 10`, nested ifs);
  switch value-variable `s.d[0] = s.d[1] = val` after the switch (vars 72 but
  via a REAL arg3 spill, 347 insns, 64-66 — rejected).
- honest producer 3 (named multi-read locals): `s16 w`, `d0` local, `v`
  reuse (m5: vars 72 via a REAL arg3 spill, 358 insns, 91 — rejected).
- types: all 31 s16/s32 combinations of i/k/v/idx/n (vars 56..96; the 72s
  are real spills of i/arg3).
- digit orderings dg1-dg5, e1-e17 single spelling edits, arg3/OT forms o1-o4,
  record-pointer forms r1-r4, a sibling-style `mode` zero holder (64).
- permuter: tmp/perm_6d8 (11.5k iters, finds = `&w`/`&p` address-escape,
  `volatile` pad, stale `*arg1` read — all rejected) and tmp/perm_6d8b
  (natural-mutation weights only, no finds).
- evidence scan (tmp/e6d8/scan.py): across the 40 func_8007352C callers the
  area after the descriptor is a separate halfword array (func_8006D3DC
  u16 rect[4] at 0x48; func_800720FC 8 halfwords), and func_8006DD94 (this
  function's caller) closed the identical untouched-slot residual under the
  OVERSIZED-LOCALS carve-out.

Adopted: OVERSIZED-LOCALS carve-out on the live digit array, `s16 d[5]`
(range d[5]..d[8], all byte-identical), FAKE-annotated with the frame proof.
