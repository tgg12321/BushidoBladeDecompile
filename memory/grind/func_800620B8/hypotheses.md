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

Frontier:
1. Owner answer to the 2026-09-26 borderline.md policy question (codegen-relation evidence for an
   aggregate merge). If YES: re-land rejected/table-merge-BA00x12-0.c with the corrected
   declaration comment (record [11] = frame 9's uv; BA30/50/58 rebuilt as absolute constants).
2. Independent prong-(a) evidence for ONE object covering 0x8009BA00..0x8009BA5F (or a wider
   same-shape run, e.g. from D_8009B8E8 / D_8009B920 to 0x8009BA5F): a function (any TU, incl. still
   INCLUDE_ASM ones) that forms one base and reaches records across the label boundaries by offset or
   stride, or a data word pointing into the run. Searched 2026-09-26: no data pointers; func_80063E10
   indexes D_8009B920 records 0..3 only; func_8006295C/func_80063084/func_80065800 use absolute
   per-record labels. Not yet searched: MOVOVL / overlay, the other text1b effect drawers' indexing
   of the B8xx records.
3. Any split-symbol spelling that relates the four addresses (would need one C object; the
   cross-symbol `(u8 *)D_8009BA00 + 0x50` pun is F4-refused, so none is known).
