# func_800620B8 — hypotheses (manual lane, slotC, 2026-09-26)

Ruled out (measured, see evidence.md table for the final-chassis receipts):
- Duplicated switch tails in any case order (3021/0312/0123/3012/0321): 52-117 on the e5 chassis.
- Split splat symbols for the sprite tables: 47 (the landable floor, candidate.c).
- `SetTransMatrix(base)` / `((MATRIX *)base)->t` inline in the loop: 65-137.
- Struct-member tag link (`prim->tag`): 22.
- One product local reused for width and height: not pursued (multi-write; Ruling 5 1(a) fails).
- Preamble orders: prim first 3; D_800A3474 read at the call 3; all three scratch pointers read up
  front 6; `SetRotMatrix(rot)` 71.
- Split symbols + per-tail table-pointer locals (frames_a/alt_a/...): 47 / 55 (cse propagates the
  constants back to the use; life stays short).
- Split symbols + four function-scope table-pointer locals set before the loop: 39 (all four
  unallocated; sv takes $s8; frame 72 vs 80) -- also a locals-for-allocator construct, not landable.

FAILED at layer-2 (2026-09-26): the 12-record `D_8009BA00[12][4]` merge (prong (a)); body in
rejected/table-merge-BA00x12-0.c. See evidence.md.

Session 2 (2026-09-26, slotC2) — ruled out, with proof (evidence.md "Session 2"):
- ANY ordinary split-symbol body: the target's one-use `$fp` = BA00 needs weighted nrefs >= 4, which
  only pre-header `(plus reg_BA00 N)` pseudos provide (one object). Measured aliases: sel_a alias 47,
  pre-loop alias 50, loop-top alias 50 (hoisted, pri 45 < sv 88), per-tail aliases 47. The original
  cc1psx reproduces both sides (split -> 47 shape; one table -> target's `$fp`/`$t0` exactly).
- Permuter on split symbols: NOT run — dominated by the proof (any find must add a second in-loop BA00
  reference, which the target's bytes rule out, or be a pun). Run it only if the proof is refuted.
- Independent prong-(a) evidence: binary-wide base+offset scan (main EXE + MOVOVL) negative; every
  indexed table in the region is indexed over exactly its own label; no census/config row; no data
  pointers; no symbol files. Only func_800620B8 references BA00/BA30/BA50/BA58.

Frontier:
1. OWNER ANSWER to the borderline.md 2026-09-26 question (+ session-2 addendum). The function is
   policy-stuck, not technique-stuck: 0/501 exists (rejected/table-merge-BA00x12-0.c, oracle-proven)
   and the split-symbol 47 is structural. If YES: re-land the merge as a record struct in
   include/game.h (TexRec-shaped {clut_x, clut_y, u, v}; TexRec is currently TU-local at
   src/text1b.c ~6448, after this function — declare the header type or move it), 12 records
   0x8009BA00..0x8009BA5F (end fixed by the u8 table at 0x8009BA60), comment citing the register
   evidence; record [11] = frame 9's uv; BA30/50/58 are rebuilt as absolute constants by reload.
   Prong (c): the four dlabels are referenced only by asm/funcs/func_800620B8.s -> nothing to keep.
2. If NO: nothing in the current binary can supply prong (a); the function waits for a ruling change
   or new evidence (e.g. another build of the game with symbols).

(Session-1 frontier items 2-3 are closed by the session-2 search and proof above.)
