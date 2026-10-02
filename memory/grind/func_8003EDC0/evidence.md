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
- fresh token local in P4's inner loop (vs the stream word w): 21
  (rejected/fresh-p4-token-21.c = s16 token, chassis y3; the u16 token and the no-local
  `D_800A87E0[i]` re-read forms are also 21); on the older k-separate chassis 13.
- fresh P4 index k instead of reusing i (P3 row counter): 5 (rejected/fresh-p4-index-5.c);
  the target's P3 row counter shares $a3 with the P4 index.
- x/z locals in P1 too: 4 (rejected/p1-xz-locals-4.c). P4 indexing grid[pos/32][pos%32]
  inline (no x/z): 34 vs 13 at the time (rejected/p4-inline-divmod-34.c).
- unk0 flag: nested if/else 41-51 (rejected/nested-if-flag-51.c), ?: 41-60, store-flag
  expression 55; `if (vz != (vx == vy)) 1 else 0` is the target's branch shape.
- max: (D >= i) ? D : i is the best; (D < i) ? i : D +1, (i < D) ? D : i +2 (chassis m*);
  `if (D < i) D = i;` stores conditionally and reloads (6 vs 5 on the no-argument chassis w1/w2).

## LANDED (2026-10-02, lane oct2-b7) — ledger closed
- A cheat-cleanup a3b491f9a (record types in include/code6cac.h; func_8003E6D8 / func_8003EB84 /
  game_GetCharData respelled), B Match 2134795df (body = tmp chassis s1 with the retyped arrays,
  i.e. no casts; body_hash a5cccb049462ad0d), queue 490eee2c3. Oracle SHA1 for A alone and A+B.
- Layer-2 PASS rv2-3EDC0-1 on both parts (A scope cheat-cleanup, B scope match), no required fixes.
  Key findings: the func_80052C10 argument is admissible as a labelled FAKE (the call is real and the
  target holds the count in $a0; the reviewer's own argument-free forms all scored 5); the i reuse is
  a labelled FAKE; w / n reuse have one semantic reading each; vz != (vx == vy) is exact semantics.
- Hygiene debt carried in the commit bodies (text1b.c prototype, D_800A3678 SVECTOR split,
  g_anim_func_table declarations, Rec4473C not unified, g_char_data symbol rows).
