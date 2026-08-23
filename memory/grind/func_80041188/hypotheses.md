# Hypothesis ledger — func_80041188

## s1 (recon, 2026-08-21) — frontier established

Chassis re-verified this session: HEAD honest floor 27 (canonical verdict C,
132/132 insns); candidate.c applied to src/text1a_pre.c scores **5** — identical
to the wip-import floor, so every banked conclusion in evidence.md remains
chassis-valid. The residual is byte-identical to the recorded 5-insn loop-2
cluster (fdiff re-run this session):

```
  addiu s3,s7,32   vs  addiu s6,s7,32     (loop2's a4+0x20 gets own reg s3)
  move  a1,s3      vs  move  a1,s6        (x2)
  move  a0,s7      vs  lw a0,88(sp)       (param-substitution stack reload)
  move  a2,s8      (ordering shift from the above)
```

All three hypotheses are grounded in the banked ALLOCDBG arithmetic
(pri = floor_log2(nrefs)*nrefs/livelen*10000) and the RA-solver session's
sufficient-pair list (evidence.md §RA-solver). The open problem is a **+1
reference lever**: every spelling found so far moves reg_n_refs by +2
(split-init) or 0 (alias copies, propagated away pre-flow).

### H1 — realize a solver pair on the split model (out2b for loop2)
- **Statement:** the real split spelling (separate out2b local for loop2)
  lands out2b in s3 (model-confirmed 12/12) but currently re-shuffles
  {loop1-out, a4, saved} into a 3-cycle (sandbox 16). One of the solver's
  sufficient PAIRS — (86 refs+1 & 86 pref+22), (79 refs+1 & 86 refs+1),
  (77 pref->23 & 86 live-8) — fixes the shuffle.
- **Mechanism:** global.c allocno priority ordering + expand_preferences
  (copy-preference to hard reg 22).
- **Probe:** `tmp/hw_split2.sh --keep`, extract the split-variant allocno
  table, compare pseudo 86's ACTUAL nrefs/pri vs the model's prediction —
  the last tested split-init spelling did NOT materialize the predicted +1
  (copy-propagated before flow). Diagnose where the ref dies, then pick the
  next pair spelling. Dumps: `pwsh tools/grinder/dump.ps1 func_80041188`,
  read .lreg/.greg for reg_n_refs, .combine for the copy-prop.

### H2 — duplicated-statement-into-arms as the missing +1 ref-lift
- **Statement:** a REAL out2- (or tbl-) referencing statement duplicated into
  two control-flow arms raises that pseudo's reg_n_refs by 1 when cross-jump
  re-merges the copies byte-neutrally — exactly the +1 no other spelling
  reaches (split-init = +2, alias = 0). Target windows: split-out2b needs
  nrefs 4 at livelen 42-76 (pri into (1052,1666)); alternatively tbl nrefs
  4→5 (1666→2083) so a 1904 split-out2 slots between tbl and the carrier.
- **Mechanism:** flow.c counts reg_n_refs before jump2's cross-jump merge;
  sanctioned family `.claude/rules/duplicated-statement-into-arms.md` — "the
  proven byte-free ref-lift for global-RA priority walls" (motion_SetMotion).
  Prereqs: byte-neutrality verified + exhaustion + FAKE annotation + reviews.
  NOTE: this function is two straight-line loop bodies; the only arms are the
  loop-exit conditionals, so the duplication site must be constructed around
  `if (i < 0x12/0x14)` — feasibility is the first thing to measure.
- **Probe:** read the rule file end-to-end first; then duplicate one real
  statement (e.g. the out2 re-init or a tbl advance) into exit-vs-fallthrough
  arms, verify build_insns==132, read .greg nrefs delta.

### H3 — restore `move a0,s7` without the parameter substitution
- **Statement:** lever 4 (param a4 at loop2's func_800523E0) buys the s6/s7
  pairing but costs the `lw a0,88(sp)` reload. A different spelling that
  keeps the carrier's pri below out2's 1333 (carrier nrefs <= 5) while
  passing the CARRIER at that call would close 2 of the 5 residual insns.
- **Mechanism:** global.c priority (carrier nrefs 7→5 was worth 1473→1052);
  solver pair (77 pref->23 & 86 live-8) names an alternative route.
- **Probe:** sweep carrier-reference-reducing spellings at OTHER a4 sites
  (evidence.md says non-func_800523E0 substitutions cost an instruction —
  those exact sites are dead; the untried surface is reducing refs via the
  out2 re-init spelling, e.g. deriving loop2's out2 from saved/base instead
  of the carrier, which also serves H1's 86 live-8).

## [s1] The wip-import floor of 5 is still valid on the current main chassis, and the residual is the same 5-insn loop-2 cluster recorded in the WIP notes.
- mechanism: chassis check: rule-era committed body (register pin + 16 rules) scores 27 honest; candidate.c is pure C and rule-independent
- probe: canonical + sandbox at HEAD (27), applied candidate.c to src/text1a_pre.c, sandbox again (5), fdiff vs build/src/text1a_pre.o reference
- result: floor 5 reproduced, 132/132 insns; fdiff shows exactly: addiu s3,s7,32 vs addiu s6,s7,32; move a1,s3 vs move a1,s6 (x2); move a0,s7 + move a1,s3/a2,s8 vs lw a0,88(sp) reorder — identical to the banked cluster
- verdict: CONFIRMED

## [s1] A sibling/duplicate analog exists that shortcuts the residual.
- mechanism: duplicate-lead scan
- probe: grep tmp/duplicates_leads.txt for func_80041188; source annotation check
- result: no entry; only a Kengo naming lead (my_hirahira/hirahira_w_ctrl), no transplantable body
- verdict: KILLED

## s2 (structural, 2026-08-21) — verdicts on the s1 frontier

### H1 (realize a solver pair on the split model) — KILLED at depth<=3
- probe: real-chassis split allocno table extracted (.lreg/.greg); full-goal
  inverse re-run (the s1 pair list was from an underspecified goal and is
  VOID — several pairs scramble tbl/i/out2b); every depth<=3 sufficient set
  requires an atom proven unspellable (see evidence.md s2 atom verdicts).
- result: split-init +1/+2 on 86 impossible (cse.c:826 make_regs_eqv
  canonicality: dest must outlive source; out2 never outlives pa4);
  staged re-init impossible (cse dest-swap + flow dead-store deletion);
  live_shrink -8 impossible (measured -1 max, bytes worsen).
- verdict: KILLED (as "one of the s1 pairs closes it"). The generalized
  hypothesis "some deeper spellable-atom vector exists" remains OPEN —
  requires the masked depth-4+ inverse (frontier).

### H2 (duplicated-statement-into-arms supplies +1) — SPLIT VERDICT
- probe: back-edge arm-dup of `offset = (*tbl)*6` in the split model.
- result: the ref-lift mechanism CONFIRMED (79: 4->5 in .lreg, flow counts
  before jump2) but byte-neutrality FAILED (cross_jump found no identical
  tail: sched1 interleaves the fall-in copy; build 134). For 86 there is NO
  dup site at all: target has no out2-referencing insn at any merge-surviving
  position, and duplicating calls is outside the family.
- verdict: KILLED for 86 (no site); mechanism CONFIRMED for 79 but
  bytes-blocked as spelled.

### H3 (restore move a0,s7 without the param substitution) — CONFIRMED, free
- probe: lever-4 revert inside the split model (loop2 523E0 takes pa4).
- result: sandbox 15 vs 16; 77 -> 7 refs (1473); param pseudo 76 dies in
  preamble; the lw a0,88(sp) residual class disappears. Lever 4 is NOT
  load-bearing in the split world.
- verdict: CONFIRMED (adopt the reverted spelling in any future split
  resolution; it does not by itself resolve the 3-cycle).

## s2 frontier

1. Extend tools/ra_solver/inverse.py with an atom-exclusion mask; re-solve
   full goal at depth 4-6 excluding {86 refs_up, 86 live±, pref_add all,
   77 refs_down}. Spellable atom inventory for the search: 79 refs+1
   (arm-dup, needs byte repair), 79 refs+2 (sym-K chain, byte-neutral),
   79/88/91/78 live moves via def position (each must be measured — the 86
   def-position probe showed C-position moves livelen far less than insn
   counting suggests), conflict_add via range overlap changes.
2. If (1) returns empty: abandon split-RA; attack candidate-5's 5-insn
   residual from the non-RA side (the s0-s8 assignment there is already
   target-correct; the diff is WHICH pseudo carries loop2's a4+0x20 —
   re-examine whether a jump2/cross-jump or sched2 shape could make the
   single-pseudo spelling EMIT addiu s3 (a fresh reload/spill-reg angle:
   reload chooses s3 if out2 spills across loop2? never probed)), and/or
   full m2c rederivation of the original variable structure.
3. Keep: lever-4-reverted split (floor 15) as the base for any split work.

## [s2] One of the s1 RA-solver sufficient pairs on the split model is spellable and resolves the {77,75,86} 3-cycle
- mechanism: global.c allocno priority order + find_reg first-free; pri = floor_log2(nrefs)*nrefs/livelen*10000
- probe: real-chassis split allocno table extracted (.lreg/.greg); inverse.py re-run with the FULL 10-pseudo goal (s1's 4-pseudo goal was underspecified and its pair list is VOID); per-atom spelling probes: split-init on 86 (folded), staged re-init out2b=out2 (dest-swapped+deleted), def-position live_shrink (moved -1 not -8, sandbox 18), arm-dup on 79 (+1 counted, 134 insns)
- result: complete depth<=3 sufficient-set list banked; every set contains an atom proven unspellable: 86 refs_up blocked by three dump-proven normalization walls, 86 live+-8 unreachable, all pref_add atoms honest-unspellable, 77 refs_down costs an instruction
- verdict: KILLED

## [s2] duplicated-statement-into-arms supplies the missing +1 reg_n_refs byte-neutrally (H2)
- mechanism: flow.c counts reg_n_refs before jump2 cross-jump re-merges the duplicate
- probe: back-edge arm dup of offset=(*tbl)*6 in the split model; .lreg refs + build_insns
- result: ref-lift REAL (79: 4->5, pri 2083) but cross_jump found no identical tail (sched1 interleaves the fall-in copy with preamble insns): build 134, sandbox 30; for 86 there is NO dup site (no out2-referencing non-call insn at any merge-surviving position)
- verdict: KILLED

## [s2] move a0,s7 can be restored without the parameter substitution (H3)
- mechanism: carrier priority stays below the window regs even at nrefs 7 (1473) in the split model; param pseudo dies in preamble
- probe: lever-4 revert (loop2 func_800523E0 takes pa4) inside the plain split; sandbox + .lreg
- result: sandbox 15 vs 16, lw a0,88(sp) residual class gone, 77=7refs/1473, same 3-cycle otherwise; lever 4 is NOT load-bearing in the split world
- verdict: CONFIRMED

## s3 (structural, 2026-08-21) — verdicts + new frontier

### [s3] A depth 4-6 sufficient vector exists over the SPELLABLE atom space (s2 frontier-1)
- mechanism: global.c priority model, masked atom inventory
- probe: tmp/grind/func_80041188/s3/masked_inverse.py on hw_split.model.json,
  full 10-pseudo goal, depth 1-4 exhaustive (137k combos) + analytic closure
  for depth 5+ (77/75/86 priorities constant under the mask; goal pins all
  higher-pri pseudos to s0-s5; ascending first-free hands 77 s6)
- result: ZERO hits; unreachable at any depth
- verdict: KILLED (split-RA axis dead end-to-end)

### [s3] candidate-5's residual closes from the non-RA side via reload/jump2 (s2 frontier-2, first half)
- mechanism: reload spill-reg choice / cross-jump on the loop2 preamble
- probe: candidate-5 .greg read — out2 IS register-allocated (s6), no spill,
  no reload of 86; the residual is pseudo-identity, not emission
- result: reload/jump2 cannot re-register a healthy allocated pseudo; the fix
  was structural (see below). The m2c-rederivation half of frontier-2 led to
  the winning structure indirectly
- verdict: KILLED as spelled; superseded by the floor-1 structure

### [s3] loop2's pointer through the REUSED dead stptr local with an out2-read re-init
- mechanism: merged stptr pseudo (10/88, pri 3409) naturally lands s3 and
  shares it across loop1-walker + loop2-pointer ranges; the copy read gives
  out2 its 4th flow ref + live 47 -> pri tie 1702 with tbl, tie breaks
  our way by allocno number; lever-4 revert restores move a0,s7
- probe: sandbox + lreg/greg dumps (see evidence s3)
- result: sandbox 1 (from 5); single residual insn move s3,s6 vs addiu s3,s7,32
- verdict: CONFIRMED (new floor; construct family: variable-reuse, FAKE
  annotation required at submission)

## s3 frontier (the last insn: move s3,s6 -> addiu s3,s7,32)

1. Combine-window search: an out2-read pair whose post-flow combine merge
   emits `stptr := pa4 + 0x20` (addiu s3,s7,32) while flow counts the reads.
   Constraint map (all measured/derived this session): cancellation must be
   invisible to front-end fold (cross-statement) AND to cse1 (preamble2 is
   its own cse block, but canon_reg + qty-fold kill same-block cancellation
   pairs) AND survive flow's uncounted dead-store deletion; combine needs
   even out2-read parity, and +2 refs demands live 59-79 (read positions
   that don't exist: preamble2 ends ~52, loop2-interior reads extend to ~88).
   Next probe: enumerate 3-insn combine shapes (P1,P2 -> C) where ONE read
   is in P-chain and the OTHER visible ref count stays 4 by REMOVING a
   loop1 ref... (loop1 refs are fixed by target bytes: def + 2 call args —
   check whether a loop1 call arg can be spelled through a fresh
   once-written-once-read named intermediate (sanctioned family) so the
   +1/-1 budget rebalances: out2 total stays 4 with one read relocated
   into the combine-deleted preamble2 pair).
2. Sub-word/alias respelling of the re-init that still READS out2's reg but
   emits addiu from s7: none found; likely nonexistent — if (1) dies too,
   the honest conclusion is that the ORIGINAL's own spelling produced the
   addiu with different flow arithmetic elsewhere (e.g. a3 or carrier refs
   differing by one somewhere invisible), and the search should sweep
   single-ref perturbations of OTHER pseudos' spellings while keeping the
   floor-1 structure: e.g. tbl's def spelling (sym-K chain, refs+2 banked
   s2), i's re-init, a1/a2 advance forms — each changes a pri and may open
   a window where out2 at 3 visible refs still lands s6 (e.g. carrier
   BELOW 714 needs carrier refs <= 3: impossible; but the TIE mechanics
   mean small live-length shifts of tbl/out2 via statement placement near
   the preamble2 boundary are one-insn levers — sweep them).
3. Permuter is CLOSED for this function (R3 cap, 2 sessions used). Manual
   sweeps only.

## [s3] A depth 4-6 sufficient vector exists over the spellable atom space of the split model (s2 frontier-1)
- mechanism: global.c allocno priority + find_reg ascending first-free, masked to spellable atoms only
- probe: tmp/grind/func_80041188/s3/masked_inverse.py on hw_split.model.json, full 10-pseudo goal, depth 1-4 exhaustive (137k combos) + analytic closure for all deeper depths (77/75/86 priorities constant under the mask, goal pins higher-pri pseudos to s0-s5, so s6 is free at 77's turn and the ascending scan hands it to 77)
- result: ZERO hits at depth <= 4; analytically unreachable at any depth; split-RA axis dead end-to-end
- verdict: KILLED

## [s3] candidate-5's residual closes from the reload/jump2 side (s2 frontier-2 first half)
- mechanism: reload spill-reg choice / cross-jump on the loop2 preamble
- probe: candidate-5 .greg read: pseudo 86 (out2) is register-allocated in s6, no spill, no reload; residual is pseudo identity, not emission
- result: reload/jump2 cannot re-register a healthy allocated pseudo
- verdict: KILLED

## [s3] loop2's pointer through the REUSED dead stptr local with an out2-read re-init (stptr = (s32) out2; as last preamble2 statement, loop2 calls take (s32*)stptr, lever-4 reverted)
- mechanism: merged stptr pseudo (10 refs/88, pri 3409) naturally shares s3 across the dead loop1-walker range and the loop2-pointer range; the copy read is out2's 4th flow ref extending live to 47 -> pri 1702.13 EXACTLY tied with tbl, tie broken by allocno number (79<86) -> tbl s5, out2 s6; lever-4 revert restores move a0,s7
- probe: sandbox + regenerated .lreg/.greg dumps; order-swap probe (10) and pa4-read re-init probe (15) bound the knife-edge
- result: sandbox 1 (from 5), 132/132; residual = one insn: move s3,s6 vs addiu s3,s7,32 at slot 71
- verdict: CONFIRMED

## s4 frontier (permuter) - RESOLVED

All three s3 frontier entries are now CLOSED by the s4 result:
1. "3-insn combine shape merging into addiu while flow counts the reads" -
   OBSOLETE, not needed. The ref and the bytes never had to come from the same
   statement: the ref can be supplied by a SEPARATE, real, consumed staging
   assignment (`out2 = (s32 *) stptr;`) in loop2, which is what the pa4-read
   chassis was missing. The whole "same honest statement" framing was the
   wrong constraint.
2. "some OTHER pseudo's flow stats differed by one; sweep statement placement
   near the preamble2 boundary" - CONFIRMED in substance, resolved by a
   different instrument: the missing one-insn worth of weighting came from the
   loop note of a single-level do { } while (0) around loop1's leading half,
   not from a statement-placement shift.
3. "one-insn spelling-class ruling-request" - NOT NEEDED. No ruling is
   required; both closing constructs sit inside pre-existing sanctioned
   families with quoted scopes and cited precedents (see self_vet.md).

No open hypotheses remain. The function is at honest distance 0 and the next
step is adversarial review + integration, not further search.
