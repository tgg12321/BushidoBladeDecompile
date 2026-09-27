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
