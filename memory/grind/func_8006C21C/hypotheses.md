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
