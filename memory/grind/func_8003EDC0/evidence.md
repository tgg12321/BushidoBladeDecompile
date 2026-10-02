# func_8003EDC0 — evidence (lane oct2-b7, 2026-10-02)

## Floor: 0/234 (sandbox --disable all, candidate.c = tmp chassis s1)
candidate.c reaches the record tables through casts ONLY so it scores against today's
`extern u8 D_800A4750[] / D_800A6690[]`; the landing form retypes those arrays
(Unk800A4750Rec / Unk800A6690Rec in include/code6cac.h) and drops the casts.
Siblings respelled onto the record types score 0 (func_8003EB84 0/143, func_8003E6D8 0/297).
`g_anim_func_table` vs `D_800F66A0` (same address): both 0.

## Decode
- P1 (pos,tok) pairs -> 16-byte records D_800A4750[n]; n (s16) -> D_800A3368.
- P2 0x68-byte records D_800A6690[n]: an Unk80101DF0Record transform node (+0x58 u8 list flag):
  xf.mat.t[0..2] built lo then |= hi<<16, xf.rot.vx/vy/vz, unk0 = vz != (vx == vy),
  g_anim_func_table[unk8](&xf.rot, &xf.mat) (func_800418D0's call shape), unk58 = 0.
- P3 grid D_800A7FE0[32][32] = -1. P4 cells -> first index into D_800A87E0, words copied
  until bit 15; D_800A3230 = max(D_800A3230, count); >= 1000 -> func_80052C10 (store to
  0x1F800400 = past scratchpad: deliberate bus-error halt; func_8003FA24 passes it a string).
- `pos % 32` / `pos / 32` are HImode-shortened (s16 pos): the remainder keeps sll/sra, the
  quotient's extension is dropped by combine (sign-bit copies).

## Measured alternatives (score; chassis)
- func_80052C10() with no argument: 5 (rejected/noarg-call-5.c, chassis w0). The target keeps
  the max in $a0 across `slti`/jal; without the call argument it lands in $v1 / $v0.
- fresh s16 counter for P2 instead of re-zeroing n: 7 (rejected/fresh-p2-counter-7.c).
- fresh u16 token local in P4's inner loop (vs the stream word w): 21
  (rejected/fresh-p4-token-21.c, chassis y3); on the older k-separate chassis 13.
- fresh P4 index k instead of reusing i (P3 row counter): 5 (rejected/fresh-p4-index-5.c);
  the target's P3 row counter shares $a3 with the P4 index.
- x/z locals in P1 too: 4 (rejected/p1-xz-locals-4.c). P4 indexing grid[pos/32][pos%32]
  inline (no x/z): 34 vs 13 at the time (rejected/p4-inline-divmod-34.c).
- unk0 flag: nested if/else 41-51 (rejected/nested-if-flag-51.c), ?: 41-60, store-flag
  expression 55; `if (vz != (vx == vy)) 1 else 0` is the target's branch shape.
- max: (D >= i) ? D : i is the best; (D < i) ? i : D +1, (i < D) ? D : i +2 (chassis m*);
  `if (D < i) D = i;` stores conditionally and reloads (6 vs 5 on the no-argument chassis w1/w2).
