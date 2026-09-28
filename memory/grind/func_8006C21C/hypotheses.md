# Hypothesis ledger — func_8006C21C (was saTan4GaugeMain)

## Ruled out (s2, 2026-09-27; receipts in evidence.md)
- `y - (row - 1)`, `y + 1 - row`, `k = row - 1` per iteration: fold reassociation / giv benefit 0.
- `- row * 4` style bevel terms (subu with +4 giv); `(2 - row*4)`; `(1 - row*2)*2` (v15a/c).
- `next++` pointer for bar 1 (extra offset-2 giv); `rec++` biv in phase 6 (offset-6 giv split);
  `recs[i].f` in phase 6 (inner-loop invariant copy).
- separate `level` variable (allocator seats j in fp, spills level): 114.
- global-access/typing/inline-helper respellings for the frame (see evidence s2 cont.).
- do-while(0) placements for the 0x80 hoist (80 combos).

## Open frontier (next worker)
1. FRAME: find the 4 phantom slots (regno in (recs, d4) => phases 1-7 temporaries or locals
   declared after recs). Instrument: `python3 tmp/func_8006C21C/cc.py <cand> --instr` prints
   FRAMEDBG spill_new_pN lines; grep the .combine dump (tmp/func_8006C21C/dump.sh + fx.py) for
   `^(insn N N N (use (reg` orphans. Untested ideas: struct-typed ctx (arg0 as a struct pointer
   with named members), Env declared at a different scope/type, loops with a non-folded entry
   test, s16-typed *memory* fields vs locals, `for` vs `do/while` for the constant-bound loops.
2. 0x80: an ordinary form in which the two else-arm constants are NOT matched movables (or are
   set twice). If none exists, file a policy-question (twice-written constant holder).
3. Tail order: row++ vs d4 load (sched2); try d-update/poly++/AddPrim statement orders and
   k/row declaration order (not yet varied).
4. Policy packaging before landing: `cells` (R9, type s.header so no cast), `idx` (R11 proof:
   fp chosen by find_reg only when s0-s7 all conflict; s6/s7 are used ONLY in phase 8, so every
   fp holder in phases 2/4/6 must be live in phase 8 => the same pseudo as level), `mode` FAKE.

## Frontier after s3 (2026-09-27, stock cc1) — floor 47 (622/622)
1. GAP 1 phantom frame slots (biggest, ~35 pts): four untouched reload spill_new slots,
   regno between recs and d4. Expand-time locals and caller-save are excluded. Lead: combine
   orphans (3->2 newi2pat + dead i2dest -> `(use (reg))` at a label). Our body has zero 3->2
   combines (logging cc1 copy: tmp/func_8006C21C/patch_comb.py + comb.sh). The only
   frame-moving leads: s16 level temp used in bar-1 compare (o4: +2 slots but spills lv, code
   differs) and `s16 mode` (+2 slots, a1 folds to const). Next: original-style struct access
   for D_800A34FC (func_8006CCC8's `s16 field` pattern) and per-phase s16 locals whose HImode
   value stays live; study sibling func_800720FC (2 SetDrawMode, 2 phantoms).
2. GAP 2 0x80 hoist: only a twice-written constant holder (t1, 39) closes it. Both writes are
   0x80, so R11 (C)(3) + Q20 per-branch clause refuses. File a policy question ONLY after
   showing no ordinary form (do-while(0) x80 placements failed; the others are untried).
3. GAP 3 row++/k++ tail order (2 insns): 360 statement orders fail; revisit after gap 1,
   because frame and allocation changes can shift sched.

## Frontier after s4 (2026-09-27)
1. The four cursor advances are not the missing-slot source: typed pointer arithmetic is
   byte-identical. Typed header/cell access is also excluded (score 59, vars unchanged).
2. A live `s16 level` comparison produces exactly two reload slots but necessarily emits five
   extra instructions. Look for a source-natural pair of HImode values eliminated later than
   combine, rather than retaining either value through a comparison/call.
3. `switch (i)` and an unsigned one-value range canonicalize to the same RTL as `i == 5`; they
   do not separate the two movable 0x80 constants. A new CFG family is required.

## Frontier after s5 (2026-09-28) — floor 41 (622/622)
1. FRAME (largest block): the four phantoms are the orphaned slt temps of the duplicated
   entry tests of the four 2-count loops after phase 2 (proof: `n = 2` bound, evidence s5).
   Find the bound/init spelling where CSE1/CSE2 cannot fold the entry test but combine folds
   it to a no-op branch. Untried: `j < n` with n assigned in the SAME block as the loop but
   through something CSE cannot see (e.g. a struct/array element, a call-free copy chain);
   a bound that is a sizeof/element-count of a local array; bounds on the pl loop too.
   Instrument: tmp/orphans.py on a -dc dump lists orphan pseudos per function.
2. r3 store order (2 hunks): the chains fix the hoist but store r3 early. Needs a target-order
   form whose per-arm 0x80 lifetime is <= 2, or two arms that do not match in
   combine_movables (different set_src rtx, n_times_set > 1, or m1->global).
3. Policy packaging before landing: `cells` (R9), `i` doubling as level (R11), `mode` (FAKE),
   `next` (second recs pointer), the 0x80 chains.

## Frontier after s6 (2026-09-28) — floor 37 (622/622, zero source-level hunks)
1. FRAME is now the ONLY gap (all scored hunks are sp offsets). Mechanism pinned (evidence s6):
   the four slots are orphan pseudos from combine 3->1 folds of a NARROW local's sign
   extension, where the local's value is known (constant, nonzero_bits 0) and it is read in a
   block CSE cannot see its set from. Proof: s16 zero holders at SetDrawMode's 5th arg
   (phases 3/5/8) + phase-4 `s.x = 0` give vars=128 and score 2
   (rejected/s16-zero-holders-frame-exact-2.c; 6 without `col`). Holders are forbidden
   frame coercion, so the task is to find the source-natural narrow value that does this.
   Leads: (a) a genuinely-used s16/u16 local initialised once near the top whose later reads
   land in s32 stores or stack args (Env x/y fields, the tw/RECT arg, a SetDrawMode flag);
   (b) a narrow value whose extension is droppable because its sign is known (lh load,
   `& mask`) read in a later block. Using one holder at ALL same-meaning sites costs code
   (block-0 sites fold in CSE; some later sites leave the holder live), so the natural form
   must only be read after block 0.
2. Not yet tried for (a): Env declared with s16 x/y locals mirrored into the struct; a RECT
   local; SetDrawMode's real PsyQ prototype (RECT *tw) with a narrow NULL-ish source.
3. If no natural producer exists, ask the owner (borderline.md policy-question): the target's
   four slots are provably narrow-holder orphans; is a named s16 constant local read only at
   those sites admissible when no other spelling reproduces the frame?

## Frontier after s7 (2026-09-28) — checkpoint 41; s6 37 rejected
1. s6 col does NOT qualify under Q20: branch-local reads never join, and the
   second bar re-stores the value already held on every feasible path. The old
   "frame is the only gap" headline is no longer the accepted frontier.
2. next/k cluster and tile/gauge rec reuse are unnecessary: combined removal,
   truthful byte-pointer cells, and both color chains still score 41/622.
3. Find an ordinary frame producer and natural-order color spelling. No holder
   whose mechanism is frame reservation is admitted. Prior suggestions to ask
   for a new exception are not part of this session's plan.
4. i/cells/mode still require full admission evidence or removal. plain-s7.c
   records a combined-removal baseline (123/605); one-at-a-time failures are not
   proof of necessity. See review-s7.md and probes/s7/ for receipts.

## Frontier after s8 (2026-09-28) — floor 41 (622/622)
Both gaps now reduce to one question: which ordinary construct makes a constant not a lone
per-site literal. Mechanisms are pinned (evidence s8); every zero-cost reproducer found is a
variable written/held with a constant, which current policy refuses.
1. FRAME: need 4 folds of a known-zero narrow value, read after a label, outside loops, at zero
   cost. Untried: narrow values that are zero by KNOWN BITS rather than by a constant init (a
   masked/shifted real value whose extension folds to 0 — e.g. a byte/halfword extracted from a
   word whose bits are provably clear); zero-valued narrow reads in the phase-3/5 blocks
   (SetDrawMode dtd/tw) with a semantic origin. Do not re-run respelling or field-type sweeps.
2. 0x80: escapes (a)-(d) in evidence s8. (c) is the only non-variable one: needs >= 5 movables
   moved from the row loop AND the j loop before the else-arm constant in insn order. The row loop
   has none today; find a natural in-loop invariant (not i-dependent in the if-arm, since CSE folds
   i == 5 there) that the original may have computed per row.
3. candidate.c now uses setXYWH for bar 1 (s8, byte-neutral); bar 2 stays explicit.
4. Policy status unchanged: i/cells/mode admissions (review-s7.md) still open.
