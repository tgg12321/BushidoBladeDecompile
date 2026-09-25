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

## Manual s2 (2026-09-25): dumps + permuter. Floor unchanged at 5/393
Scratch: tmp/f1be20s2/ (mk.py = splice + instrumented-cc1 `-da` dumps into
tmp/rtl/<tag>/, ex.sh = print the extraction window, pdiff.py = token diff of
permuter outputs, mkperm.sh = hand-built permuter workspace).

**Allocation dump (BB2_ALLOC_DEBUG / BB2_FINDREG_DEBUG, rejected 0-form):**
`shift` = pseudo 75 (13 refs, livelen 173, ord 12) conflicts with hard regs
{v0,v1,a0,a1,a2,sp}. a1 comes from `out` (pseudo 76, the loop's accumulator)
and a2 from the loop pointer (pseudo 479). Neither is live during the pad
extraction. So a3 is the result of the pseudo living INTO THE LOOP. Nothing in
the extraction window blocks a1/a2. P (arg0<<4, the arg0*0x44C multiply's
first step) = pseudo 95 in a2, dies at the copy. No `regs_someone_prefers`
entry is involved (the someone_prefers set is empty).

**CSE extent (f.cse):** the struct copy + extraction + nibble if/else are one
followed-jump path, insns 57..167 (also re-run as 97..167). The multiply sits
at insn ~61 in the same path. make_regs_eqv keeps a copy `H = P` only if H has
a reference before the path start (57) or after its end (167), and its last
reference comes after P's.

**Why no single-write, single-role H can produce `move a3,a2`:**
1. All refs inside 57..167: P stays canonical and the copy is deleted (cand, 5).
2. Ref before 57, which means a write in the first block. Any early value is
   orphaned once CSE rewrites the later computation in terms of P, so flow
   deletes it and H becomes block-local. local-alloc then gives it a1 (L3,
   d1: a dead init, before or after the struct copy). If the early value is a
   copy of arg0 and the later statement is `half *= 16` (e1), CSE can't link
   H to P in the new block, so it emits `sll a2,a2,4` instead of a move.
3. A read after 167 makes H live across the loop, where it conflicts with N
   (the nibble shift, a3 in the target). Then H cannot be a3.
4. A write after 167 is a second write. The permuter's first find is exactly
   this (output-25-1: `half = 0x2000;` reused in the player-1 remap), and it
   is Ruling 5 inadmissible.
5. d2 (diagnostic: buf[3] moved past the if/else to make H global without
   entering the loop): a global H takes the lowest free reg (a0 there). A
   global H does not drift to a3 unless a1/a2 conflict.
So the target's bytes need ONE pseudo that holds arg0*16 during the
extraction and is live in the loop, i.e. the two-role `shift`, or a
post-loop second write. Both are refused (Ruling 5 1(a)/(b)/(c)/(e)/(f);
Ruling 6 covers record pointers only; Ruling 4 covers consecutive splits
only, and a consecutive split stays inside 57..167).

**Permuter** (tmp/perm_1be20_a, default weights, -j2, from candidate.c,
base 120, ~11.8k iterations, ~16 min, stopped and harvested). Every
improving find was one data flow, the nibble-shift local carrying arg0*16
during the extraction:
- output-0-2: `half = arg0 * 16; shift = half;`, with buf[0] read via `shift`
  and buf[1..3] via `half`.
- output-0-1: `shift = arg0 * 16;` plus noise.
- output-25-1: a post-loop `half = 0x2000;`.
- output-45-1: `shift = 4; shift = arg0 * shift;` plus a post-loop half write.
- The rest (35/70/105/110/115) are garbage: reads before writes, and `half`
  borrowed as the colour byte.
All of these are two-role reuse, the family layer-2 has already refused.
There were ZERO single-role finds. This independently confirms the dump
proof above.

**Frontier for the next session:** under the current rulings the honest
floor is 5, and the gap is structural. The only closing forms found are
two-role reuse, rejected by layer-2 on 2026-09-25. Unexplored: a whole-function
restructure in which the loop reads a variable the programmer genuinely set to
arg0*16 first. No such role exists in the function's semantics (the loop only
uses the nibble shift), so this is low-probability. An owner ruling on the
`shift` reuse (one "per-player field shift" variable, re-set per field width)
is the other path. It is a candidate policy question for the orchestrator or
owner. s2 did not file it and did not self-authorize it.
