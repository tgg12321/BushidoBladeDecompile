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

## s5 (synthesis, 2026-08-22) — verdicts

The s4 frontier was "RESOLVED"; the layer-1 FAIL on the do-while(0) wrap
re-opened it. s5 re-derived the whole allocation problem from dumps with a
corrected priority formula and closed four levers.

### [s5] The banned wrap's loop-note weighting is reachable from a well-formed C loop
- mechanism: flow.c weights reg_n_refs by loop_depth, which is driven by
  NOTE_INSN_LOOP_BEG/END; a real do/while/for emits those notes, a goto-loop
  does not
- probe: loop1 as `do {...} while (i < 0x12)`; loop2 as `do {...} while
  (i < 0x14)`; both; sandbox + .lreg for each
- result: loop1-real 132 insns / sandbox 28 (pa4 carrier 7->9 refs = 2872 rises
  FASTER than out2 3->7 refs/41; i and tbl swap seats); loop2-real 135 insns
  (loop.c adds 3 insns on a note-marked loop); both-real 135 insns / 28
- verdict: KILLED. Loop notes move the allocation AWAY from target here; the
  wrap worked only because it weighted a HALF-body, which no C loop can express
  without changing the CFG.

### [s5] tbl (79) refs+2 (the banked sym-K chain) opens the seating
- mechanism: cse1's cost gate keeps a chain whose fold-back target is a 2-insn
  lui/addiu symbol constant; flow counts both refs; combine re-merges
- probe: `tbl = (s32*)((u8*)D_80094CFC - 0x10); tbl = (s32*)((u8*)tbl + 0x10);`
  on the pa4-read+staging chassis; then 24 variants (6 preamble permutations x
  4 advance positions)
- result: the lift is REAL and byte-neutral (132 insns, tbl 4->6 refs in .lreg)
  but pins tbl at 2553.2 > i (2474.2) so tbl steals s4. Required window
  (2381.0, 2474.2) = live 49-50; tbl's live is scheduler-pinned to 45-47 in
  all 24 variants
- verdict: KILLED as a closer (mechanism CONFIRMED and now banked as a reusable
  byte-neutral +2 ref-lift for symbol-constant pseudos)

### [s5] i (78) and stptr2 (90) can be ref-lifted to re-open tbl's window
- mechanism: same split-init accumulation; target order would be
  90 (8/48=5000) > 87 (3409) > 78 (10/97=3092) > 79 (2553) > 86 (2381) > 77
- probe: `stptr2 = saved + 0x700; stptr2 = stptr2 + 0x50;` and
  `i = 0x10; i = i + 2;` on the tbl-lifted chassis; .lreg refs read
- result: reg_n_refs UNCHANGED (stptr2 still 6, i still 8); both chains fold
  in cse1 (fold-back is a cheaper 1-insn addiu) and flow deletes the dead
  first store uncounted. sandbox 12, identical to the unlifted form
- verdict: KILLED. Split-init ref-lifts are available ONLY on
  symbol-constant-based pseudos.

### [s5] Some staging position gives out2 a window-landing pri byte-free
- mechanism: reg_live_length grows with the distance from the staging def to
  its use; the copy is coalesced away post-flow when def and use are adjacent
- probe: 6 positions of `out2 = (s32*)stptr;` (preamble2, loop2 top, after
  func_80044DE4, mid-loop2, and two both-uses variants), sandbox + .lreg
- result: only the immediately-before-the-call position is byte-free (5/42 =
  2381, sandbox 9). Every position far enough to lengthen the live range
  materialises `addu s6,s3,zero` (133 insns). The loop2-top variant reaches
  out2 6/81 = 1481.5 and **EXACTLY target's register dispositions**, sandbox 3
- verdict: KILLED as spelled; the 133-insn form is banked as the sharpest
  statement of what is missing (one free instruction-slot for the copy)

### [s5] Dropping / splitting the pa4 carrier changes the see-saw
- probe: use the `a4` parameter at all six sites (V1); carrier for loop1 only
  and the parameter for loop2 (V2)
- result: V1 132 insns / sandbox 10 but a completely different allocno world
  (out2 absorbs the 10-ref merged pseudo; param 7/190 = 736.8); V2 134 insns
- verdict: KILLED

## s5 frontier (the three strongest, for the next ladder pass)

1. **A second byte-free flow-counted `out2` read in the MID block (block 2).**
   This is now provably the whole remaining problem: with it, out2 reaches
   4 refs / live 48-54 (pri 1666-1481, inside the (1473.7, 1702.1) window)
   while MID's `stptr = (s32)(((u8*)pa4)+0x20)` still emits target's
   `addiu s3,s7,32`.
   mechanism: global.c allocno priority; MID is its own cse basic block, so
   cse1 has NO qty knowledge that out2 == pa4+0x20 there (s3 evidence) — which
   means an `out2`-reading expression in MID will NOT be constant-folded the
   way the same expression is inside block 0.
   next probe: enumerate expressions that read out2 in MID and whose emitted
   form is one of MID's six target insns, exploiting the missing qty knowledge
   (e.g. spellings where cse's cost gate prefers keeping the out2 read over
   re-materialising pa4+0x20). Read the .cse/.combine dumps for the MID block
   FIRST — the s3 "everything folds pre-flow" conclusion was drawn on block 0
   semantics and has never been re-measured for block 2 with the corrected
   arithmetic.

2. **Kill the one materialised `addu s6,s3,zero` in the loop2-top staging form
   (rejected/staging-position-sweep-133insn.c, sandbox 3, dispositions EXACT).**
   mechanism: the copy survives because out2's staged value is live across
   loop2's back edge, so post-flow copy propagation cannot fold it into the
   single use at func_800523E0.
   next probe: read .combine/.jump2/.greg for that form and identify which pass
   declines the coalesce; then try spellings that keep the long live range but
   make the def and the use copy-propagable (e.g. staging a value that loop2
   already recomputes, or a staging def placed on the loop2 back edge so the
   first iteration uses the preamble value — semantically identical, since
   `out2` and loop2's `stptr` hold the same `pa4 + 0x20`).

3. **Widen the window instead of moving out2: lower the pa4 carrier below
   out2's 3-ref pri.** On the pa4-read chassis out2 sits at 714.3 and the
   carrier at 1473.7 (7 refs / live 95); the carrier would have to reach
   <= 714 (live > 196, or refs <= 3 with a short live). Measured carrier
   variants: 6 refs/95 = 1263 (out2-read chassis), 7/94, 7/96, param-only
   7/190 = 736.8 (V1, tantalisingly close to out2's 714.3 — one more live-shift
   or one fewer param ref would invert them).
   next probe: on the V1 (no-carrier) chassis, re-derive the FULL disposition
   goal from scratch with the corrected formula — s1-s4 never analysed that
   world, and V1 already scores 10 at 132 insns.

## [s4] The banned do-while(0) wrap's loop-note ref weighting is reachable from a well-formed C loop (loop1 and/or loop2 spelled as a real do/while).
- mechanism: flow.c weights reg_n_refs by loop_depth, driven by NOTE_INSN_LOOP_BEG/END, which only a real C loop emits; a goto-loop emits none.
- probe: loop1 as do{...}while(i<0x12); loop2 as do{...}while(i<0x14); both; sandbox --disable all plus .lreg refs/live for every callee-saved pseudo.
- result: loop1-real 132 insns / sandbox 28 (carrier 7->9 refs = pri 2872 rises faster than out2 3->7 refs at live 41; i and tbl swap seats). loop2-real 135 insns (loop.c adds 3 insns on a note-marked loop). both-real 135 insns / 28.
- verdict: KILLED

## [s4] The banked 'tbl (79) refs+2 sym-K chain' opens the tbl-vs-out2 seating.
- mechanism: cse1's cost gate keeps a chain whose fold-back target is a 2-insn lui/addiu symbol constant; flow counts both refs before combine re-merges them.
- probe: tbl = (s32*)((u8*)D_80094CFC - 0x10); tbl = (s32*)((u8*)tbl + 0x10); on the pa4-read+staging chassis, then 24 variants (6 preamble permutations x 4 advance positions), .lreg each.
- result: The lift is REAL and byte-neutral (132 insns, tbl 4->6 refs) but pins tbl at pri 2553.2 > i (2474.2), so tbl steals s4. Required window is (2381.0, 2474.2) = live 49-50 at 6 refs; tbl's live length is scheduler-pinned to 45-47 in all 24 variants. sandbox 12.
- verdict: KILLED

## [s4] i (78) and stptr2 (90) can be ref-lifted by split-init accumulation to re-open tbl's window (order 90 5000 > 87 3409 > 78 3092 > 79 2553 > 86 2381 > 77 1473).
- mechanism: same split-init accumulation that lifted tbl; flow counts before combine merges.
- probe: stptr2 = saved + 0x700; stptr2 = stptr2 + 0x50; and i = 0x10; i = i + 2; on the tbl-lifted chassis; .lreg reg_n_refs read.
- result: reg_n_refs UNCHANGED (stptr2 still 6, i still 8), sandbox identical (12). Both chains fold in cse1 because the fold-back is a cheaper ONE-insn addiu, and flow then deletes the dead first store UNCOUNTED. Split-init ref-lifts exist only on symbol-constant-based pseudos.
- verdict: KILLED

## [s4] Some position of the s4 out2 staging store gives out2 a window-landing priority while staying byte-free.
- mechanism: reg_live_length grows with the def-to-use distance; the copy is coalesced away post-flow only when def and use are adjacent.
- probe: 6 staging positions (preamble2, loop2 top, after func_80044DE4, mid-loop2, and two both-uses variants); sandbox plus .lreg plus objdump slot diff.
- result: Only the immediately-before-the-call position is byte-free (out2 5/42 = 2381, sandbox 9). Every position far enough to lengthen the live range materialises one addu s6,s3,zero (133 insns). The loop2-top variant reaches out2 6/81 = 1481.5 and EXACTLY target's register dispositions, sandbox 3, defect = that single extra insn at slot 73.
- verdict: KILLED

## [s4] Dropping or splitting the pa4 carrier changes the see-saw in our favour.
- mechanism: the carrier's 7 refs / live 95 = pri 1473.7 is the floor out2 must clear.
- probe: V1 = use the a4 parameter at all six sites; V2 = carrier for loop1, parameter for loop2.
- result: V1 132 insns / sandbox 10 but a completely different allocno world (out2 absorbs the 10-ref merged pseudo; the param scores 7/190 = 736.8). V2 134 insns.
- verdict: KILLED

## [s4] The priority formula pri = floor_log2(reg_n_refs)*reg_n_refs/reg_live_length*10000 with floor_log2(3)=1 predicts the seating on every chassis, so a 3-reference out2 can never outrank the carrier.
- mechanism: GCC 2.7.2 global.c allocno_compare ordering plus find_reg's ascending first-free scan.
- probe: six chassis dumped (.lreg refs/live plus .greg dispositions): wrap-0, A_nowrap-9, P2-3, S15-10, R3-15, F1-1; predicted vs actual seating compared each time.
- result: 6/6 correct. floor_log2(3)=1 means out2 at 3 refs scores 714.3 (not the 1428 the s2/s3 notes assumed) and would need live <= 20 to clear the carrier's 1473.7. out2 therefore needs >= 4 refs unconditionally.
- verdict: CONFIRMED

## s6 (forensics, 2026-08-23) — results and the new frontier

KILLED (each with the pass named from an instrumented-cc1 dump):

- **H-s6-1 — "a second byte-free, flow-counted out2 read exists in the MID block"
  (s5 frontier #1).** KILLED. `out2 = pa4+0x20; stptr = (s32) out2;` in MID is
  dest-swapped by cse1 (cse.c cse_insn) into `stptr = pa4+0x20; out2 = stptr;`,
  out2 is dead, and flow.c's dead-store elimination deletes the copy before
  reg_n_refs is counted: allocation bit-identical to the plain pa4 chassis
  (out2 3/42/714), sandbox 15.
- **H-s6-2 — "the loop2-top staged copy can be made combine-propagable"
  (s5 frontier #2).** KILLED as a family, not just as a spelling. cse1 decides
  the fate of every staged copy: spell the LAST pointer use `out2` and cse1
  pulls the earlier use onto it too (two consumers -> combine's try_combine
  cannot delete the producer -> +1 insn, the known 133-insn form); spell any
  earlier use `out2` and cse1 propagates it back to stptr (dead store -> flow
  deletes it uncounted). The only byte-free COUNTED position is a def->use span
  of ~1 insn (5 refs / live 42-44 / pri 2381, sandbox 9); every longer span
  steps over the second func_8004A348 and flips cse1 into the materialising
  branch.
- **H-s6-3 — "the carrier-free V1 world has a reachable target seating"
  (s5 frontier #3).** KILLED. V1's tail order is a3 808 (s6) > a4 736 (s7) >
  out2 714 (fp); target needs out2 > a4 > a3, i.e. TWO inversions where the
  pa4-carrier chassis needs ONE. V1 is strictly harder.

CONFIRMED:

- **H-s6-4 — the seat rule is priority order alone.** BB2_FINDREG_DEBUG on
  pseudos 73/77/86 shows empty `own_copy_prefs` and empty `someone_prefers`;
  MIPS defines no REG_ALLOC_ORDER. Seats = lowest-numbered free non-conflicting
  callee-saved register, taken in allocno_compare order. The s5 window
  (1473.7, 1702.1) for out2 is therefore exact and complete.
- **H-s6-5 — the missing out2 reference is post-flow in the ORIGINAL too.**
  Target's s6 carries only three materialised references and never appears in
  loop2, so the original C's 4th+ out2 reference was deleted after flow counted
  it. Only combine.c can do that in this pipeline.

### Frontier for the next session

1. **The one existence proof of a counted-then-deleted out2 copy has never been
   dumped.** rejected/s4-form-minus-banned-wrap-floor9.c (staging immediately
   before its single last use) reaches out2 5 refs / live 42 BYTE-FREE — i.e.
   there cse1 did NOT propagate the single-use copy away, and combine deleted it
   after flow counted it. That contradicts the cse1 behaviour measured in
   H-s6-2 for every other position. mechanism: unknown cse.c condition (candidate:
   the copy's source qty is invalidated, or the use is inside the call-argument
   setup sequence so cse sees the reg reference after `invalidate_for_call`).
   next probe: dump .cse/.combine for that form and diff the loop2 region against
   s5/va/*; if the sparing condition is a reproducible property (rather than
   adjacency), reproduce it with a ~20-insn def->use span => 5 refs / live 59-67
   => pri 1481-1666, inside the window, byte-free => match.
2. **Attack the CONFLICT set instead of the priority order — no ref lift needed.**
   Every allocno except stptr2 (90) carries a hard-reg conflict with reg 16
   ($s0) in this function (BB2_FINDREG_DEBUG `conflicts:` lines). If the pa4
   carrier could be made to conflict with hard reg 22 ($s6) the same way, it
   would take s7 at its existing pri 1473 and out2 would take s6 at 714 with the
   C otherwise unchanged — target seats with NO reference lift at all.
   mechanism: global.c global_conflicts records hard-reg conflicts from hard regs
   live across the allocno's range. next probe: find WHAT makes reg 16 live in
   this function (grep the .greg/.flow dumps and global_conflicts' mark_reg_live
   path), then ask whether any legitimate C construct puts a value in a
   callee-saved hard reg across the carrier's range (note: `register ... asm()`
   pins are BANNED — this must be an ordinary-C mechanism or it is not a lever).
3. **Rederive the source shape under the H-s6-5 constraint.** Every chassis in
   the ledger assumes the merged `stptr` (loop1 walker re-init as loop2's
   pointer). The original must additionally have carried a combine-deletable
   out2 mention; a different loop2 pointer structure (separate local, different
   loop1/loop2 split, or the a4 store/reload shape) may supply it naturally.
   next probe: rederive from target asm + m2c with the explicit constraint
   "out2 has >= 4 flow refs and exactly 3 materialised ones", rather than
   perturbing the existing body.

## [s5] A second byte-free, flow-counted out2 read exists in the MID basic block (s5 frontier #1): 'out2 = pa4+0x20; stptr = (s32)out2;' gives out2 +2 refs at flow time while combine merges the pair into target's single 'addiu s3,s7,32'.
- mechanism: cse1 was believed to have no qty knowledge in MID (block 2), so the chain should survive to flow; combine.c would then merge producer+copy.
- probe: Installed the form, ran the instrumented cc1 (BB2_ALLOC_DEBUG=1) and traced reg 86 through .rtl/.jump/.cse/.loop/.cse2/.flow/.combine (tmp/grind/func_80041188/s5/e1/).
- result: Bytes are perfect (132 insns, target's addiu s3,s7,32) but the allocation is bit-identical to the plain pa4 chassis: out2 3 refs / live 42 / pri 714, sandbox 15. .rtl has '86 = 77+32' then '87 = 86'; .cse shows cse1 DEST-SWAPPING the pair into '87 = 77+32' then '86 = 87' (cse.c cse_insn rewrites the producer's destination when its original dest dies in the copy); out2 is then dead and flow.c's dead-store elimination removes the copy BEFORE reg_n_refs is counted. The lift is uncounted, not merely byte-free.
- verdict: KILLED

## [s5] The materialised 'addu s6,s3,zero' of the 6-refs/live-81 loop2-top staging form can be eliminated while keeping out2 inside the priority window (s5 frontier #2), e.g. by letting the staged pointer feed a different loop2 use so the copy becomes combine-deletable.
- mechanism: combine.c try_combine deletes a producer only when its destination dies at the single use it substitutes into; s5 attributed the survival to the value being live across loop2's back edge.
- probe: Read the .combine dump of rejected/staging-position-sweep-133insn.c (insn 171 '86 = 87' has a LOG_LINK consumer at insn 254 AND a REG_DEAD consumer at insn 263 = two consumers), then built and dumped the never-tried use-swap variant (copy at loop2 top consumed by the second func_8004A348, func_800523E0 keeping stptr) in tmp/grind/func_80041188/s5/va/.
- result: The family is closed, not just this spelling. cse1 decides the fate of every staged copy: spell the LAST pointer use 'out2' and cse1 pulls the earlier stptr use onto reg 86 as well (two consumers, combine cannot delete the producer, +1 insn = the known 133-insn form); spell an earlier use 'out2' and cse1 canonicalises it back onto reg 87 (loop2 uses of reg 86 go .rtl 2 -> .cse 1 -> .flow 0), the copy becomes a dead store and flow deletes it uncounted, giving an allocation bit-identical to the pa4 chassis. The only byte-free COUNTED staging position remains the s5 one (def->use span ~1 insn, out2 5 refs / live 42-44 / pri 2381, sandbox 9); every longer span necessarily steps over the second func_8004A348 and flips cse1 into the materialising branch, so 5 refs/live 59-67 and 6 refs/live 71-81 are both unreachable byte-free.
- verdict: KILLED

## [s5] The carrier-free world (V1: the a4 parameter used at all six sites) has a reachable target seating, since the parameter (736.8) and out2 (714.3) are only ~22 priority points apart (s5 frontier #3).
- mechanism: Same global.c priority ordering but a different allocno set; a single live-shift or one fewer parameter reference would invert them.
- probe: Built V1 and ran BB2_ALLOC_DEBUG (tmp/grind/func_80041188/s5/v1/stderr.txt).
- result: Tail order measured: a3 4 refs/99 = 808 -> s6, a4 7 refs/190 = 736 -> s7, out2 3 refs/42 = 714 -> fp. Target needs out2 -> s6, a4 -> s7, a3 -> fp, i.e. the order out2 > a4 > a3: out2 must pass BOTH, and a4 must additionally pass a3 (it is already 72 points below it). That is TWO inversions where the pa4-carrier chassis needs ONE - V1 is strictly harder, not closer.
- verdict: KILLED

## [s5] Register seating in this function is decided by something richer than allocno priority order (copy preferences / someone_prefers / an allocation order), which would open a lever axis not requiring any change to out2's reference count.
- mechanism: global.c find_reg overrides best_reg from hard_reg_copy_preferences / hard_reg_full_preferences and skips regs in regs_someone_prefers or outside regs_used_so_far during pass 0.
- probe: BB2_FINDREG_DEBUG on pseudos 73 (a1), 77 (carrier) and 86 (out2) on the pa4-read chassis, plus a source read of tools/gcc-2.7.2/global.c (allocno_compare, find_reg) and a check that mips.h defines no REG_ALLOC_ORDER.
- result: own_copy_prefs and someone_prefers are EMPTY for all three; pass 0 never succeeds (every callee-saved reg is excluded) and pass 1 takes the lowest-numbered non-conflicting register. Seats are therefore a pure function of allocno_compare order, which makes the s5 window pri(out2) in (1473.7, 1702.1) exact and complete. One usable side-fact: every allocno except stptr2 carries a hard-reg CONFLICT with reg 16 ($s0) - conflicts, unlike preferences, do bite in this function.
- verdict: CONFIRMED

## [s5] The original C's 4th out2 reference was materialised as an instruction we simply have not spelled yet.
- mechanism: n/a - counting argument against the target's own register file.
- probe: Counted every $s6 occurrence in asm/funcs/func_80041188.s and mapped the full callee-saved seating.
- result: Target's s6 carries exactly THREE materialised references (def 'addiu s6,s7,0x20' plus two loop1 'addu a1,s6,zero') and never appears in loop2, so 3 refs at live ~42 = pri 714 would seat it in $fp. Since the seats are pure priority order, the original's out2 must have carried >= 4 flow-counted references of which only 3 survived into the bytes: the extra reference was deleted AFTER flow counted reg_n_refs, and combine.c is the only pass in this pipeline that can do that. The remaining search is precisely 'which combine-deletable out2 mention lands (refs, live) inside the window'.
- verdict: CONFIRMED
