# func_8001BE20 — hypotheses / ruled out (manual s1, 2026-09-25)

Scratch variants: tmp/f1be20/v*.c (old chassis), L*.c (landing chassis).

## Ruled out
- Pointer loop over buf (sltu + hoisted -241): v1.
- `flag` local + if, then store: 45; ternary rhs: 44. Single `&&` expression: 33.
- Colour value read straight from the globals in each test: 73/79.
- Arm-scope `u8 r, g, b;`: 43. Single shared `col`: register swap in the
  mode-6 arm (and it is an inadmissible multi-write anyway).
- s32 pad words: sra instead of srl in the player-1 remap (8 hunks).
- Nibble shift as a ternary: worse in every chassis (L4 8, L5 20).
- Separate once-written `half` for the pad shift, before or after the struct
  copy (v5/v6): no `move`, 5.
- Dead defensive init of `half` (L3): move restored in a1, not a3: 5.
- cc1psx: not closer (33 vs 5).

## Frontier (the `shift` reuse was refused by layer-2, 2026-09-25)
The residual is exactly one copy `move a3,a2` and its four srlv readers. The
copy survives only if the pad-shift variable is mentioned outside its CSE block
(cse.c make_regs_eqv), and it sits in a3 only if the pseudo is global and live
into the loop. Untried: decomp-permuter from candidate.c (expect only
reuse/dead-use finds, which are inadmissible, but it has not been run);
spelling_enum over the pad-extraction block; a different whole-function
structure in which the pad extraction and the nibble loop share a genuinely
single-role per-player bit-offset value.

Why the obvious honest levers cannot work (reasoned from the target, 2026-09-25):
the move's destination is a3, which in the loop holds the nibble shift. A
block-local copy gets a1 (L3). A global copy whose only link is the copy from
a2 would prefer a2, which deletes the move. So a3 needs a pseudo that is live
into the loop, where a2 is the loop pointer. Checked and ruled out: deriving the
nibble shift from the pad shift (`shift >> 2` or `/ 4` emits srl/sra, but the
target has `sll a3,s1,2`); computing `half` before the unk_06 test (no move,
and the move would land in the wrong block). A pad-extraction helper does not
fit either: there is no call, and an inline parameter copy is CSE'd away like
`half`.
