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

## [s7] The floor-9 form's staged out2 copy is spared by cse1 because of a reproducible condition that could be reproduced at a ~20-insn def->use span (s6 frontier #1).
- mechanism: an unidentified cse.c condition (candidates: source qty invalidated between def and use; the use sitting inside the call-argument setup after invalidate_for_call).
- probe: instrumented-cc1 dump of rejected/s4-form-minus-banned-wrap-floor9.c (tmp/grind/func_80041188/s6/f9/*), tracing reg 86 through .rtl/.cse/.flow/.combine and comparing against the s5/va dumps.
- result: The condition IS reproducible but it is not new and it does not scale. cse.c make_regs_eqv makes the copy's DEST canonical whenever its last textual reference outlives the source's; in the floor-9 form the copy sits after loop2's other pointer use, so 86 is canonical, cse1 has nothing to rewrite, flow counts def+use (5 refs / live 42) and combine deletes the copy at its single consumer. The same rule forces the copy to sit between the last-but-one and the last pointer use, so the def->use span is ~1 insn; a ~20-insn span necessarily steps over the second func_8004A348, which puts a second consumer on the copy and makes it materialise (+1 insn).
- verdict: KILLED (the mechanism is named; the lever does not exist)

## [s7] The seats can be fixed with no ref lift by giving the pa4 carrier a hard-reg conflict with reg 22 ($s6), the way every allocno here conflicts with reg 16 (s6 frontier #2).
- mechanism: global.c global_conflicts records a hard-reg conflict for every hard reg live across an allocno's range; find_reg excludes it in both passes.
- probe: identified the source of the reg-16 conflicts (local-alloc block-local quantities), read tools/gcc-2.7.2/local-alloc.c find_free_reg, and cross-checked the target's own register usage in asm/funcs/func_80041188.s.
- result: The reg-16 conflict comes from loop1's `offset` temp - a block-local quantity live across the first func_8004A348, so it needs a callee-saved register - and find_free_reg scans hard regs STRICTLY ASCENDING (MIPS defines no REG_ALLOC_ORDER; the only override, qty_phys_copy_sugg, is populated solely from hard-reg copies, i.e. call args/returns, all call-clobbered). Reaching hard reg 22 would require six further simultaneously-live call-crossing block-local quantities in a region overlapping the carrier and a3 but not out2, and the first of them would take reg 16 and displace stptr2 from $s0. The target's asm shows the identical $s0 loop1 temp, so the original had the same single conflict.
- verdict: KILLED

## [s7] The tbl sym-K ref lift becomes usable if `i` outranks the lifted tbl, which the never-swept position of `i = 1` can supply.
- mechanism: pri = floor_log2(refs)*refs/live*10000; i at 8 refs scores 2474 at live 97 and 2500 at live 96, tbl at 6 refs scores 2553 at live 47 and 2500 at live 48. Seats are pure priority order (s6), so moving i's def relative to tbl's def flips their order.
- probe: 30 (tbl-advance position x i-def position) variants, instrumented-cc1 allocno tables for each, sandbox on the 10 that seat correctly; plus 5 variants that move tbl's symbol init out of the declaration into a statement after `i = 1;`.
- result: CONFIRMED as a seat fix. With `i = 1` placed after at least two preamble statements, i is 8/96 = 2500.0 and tbl is 6/48 = 2500.0, an exact tie broken our way by allocno number, and EVERY callee-saved disposition equals target for the first time in the ledger (sandbox 4, 132 insns, 10 spellings). But the seats are anti-correlated with the preamble EMISSION order: the def that comes first in the C is emitted first (equal INSN_PRIORITY, sched1 LUID tie-break) and also gets live+1, so the i-first order that matches target's `sw s4/addiu s4` before `sw s5/lui s5` gives i 2474 < tbl 2553 and tbl steals s4 (sandbox 12).
- verdict: CONFIRMED as the best structure so far (sandbox 4); the emission-order half is the new residual.

## [s7] A loop-carried pseudo's reg_live_length can be moved by repositioning its uses inside the loop.
- mechanism: flow.c reg_live_length counts insns where the reg is live; s5 had recorded tbl's live as "scheduler-pinned 45-47" without a mechanism.
- probe: 16 positions of loop1's `tbl++` (t01..t16), instrumented allocno table for each.
- result: bit-identical tables in all 16 (tbl 6 refs / live 47). A pseudo live across the back edge is live over the entire loop body, so in-loop position is irrelevant; only defs/uses outside the loop move the number. The whole "advance position" axis is retired.
- verdict: KILLED

## Frontier for the next session

1. **A byte-free +1 reference on `i` (78) in the i-FIRST order closes the
   all-target-seats chassis.** i needs >= 9 flow refs (3*9/97 = 2783 > tbl's
   2553) and <= 11 (3*11/97 = 3402 < stptr's 3409); everything else in
   rejected/tbl-symK-ilate-alltarget-seats-emission-swapped.c is already target,
   including the 4 preamble insns that the i-late spelling gets wrong.
   mechanism: flow.c counts reg_n_refs before combine runs, so any i mention
   deleted by combine (not by cse1 or by flow's dead-store elimination) is a
   free +1. Known blockers: preamble uses of i are constant-propagated (i is
   literally 1 there); split-init ref lifts only work on symbol-constant
   pseudos (s5 law); dead stores are deleted uncounted by flow (s2 law).
   next probe: enumerate combine-deletable i mentions in LOOP1/LOOP2 (uses
   inside a loop cost nothing in live length - the s7 invariance law - and are
   not constant-foldable there): e.g. an exit test spelled so the compare's
   operand is produced by a separate insn combine folds in (`if (i - 0x12 < 0)`,
   `if ((s32)(i - 0x14) < 0)`), or an address/offset expression in the loop body
   that consumes i and is folded into an existing addiu. Measure .lreg refs for
   78 and the insn count for each.
2. **Attack the preamble emission order directly instead of the seats.** In the
   i-late (target-seat) spelling the only defect is that sched1 emits the tbl
   pair before the i pair. sched1 ranks by INSN_PRIORITY and breaks ties by
   LUID, so giving `li i,1` a dependent inside block 0 would raise its priority
   above the `la tbl` - but every block-0 use of i is constant-propagated by
   cse1 before flow/sched see it. next probe: look for a block-0 consumer of i
   that cse1 CANNOT fold (i.e. one whose value is not the literal 1 at that
   point) yet costs no bytes, or a dependence edge that lengthens the tbl pair's
   path instead of shortening i's.
3. **Rederive under the joint constraint** (unchanged from s6 frontier #3, now
   sharper): the original has out2 with >= 4 flow refs / exactly 3 materialised
   ones AND `i` outranking a 6-ref tbl - or, more likely, it never lifted tbl at
   all and out2's 4th reference lands in (1473.7, 1702.1) some other way. The
   tbl sym-K chain is our invention and would face the cheat checklist; a
   rederivation that removes the need for it is worth more than one that keeps
   it.

## [s6] The floor-9 form's staged out2 copy is spared by cse1 through a reproducible condition that could be reproduced at a ~20-insn def->use span, yielding out2 5 refs / live 59-67 byte-free (s6 frontier #1).
- mechanism: An unidentified cse.c condition was hypothesised (source qty invalidated between def and use, or the use sitting inside the call-argument setup after invalidate_for_call).
- probe: Instrumented-cc1 dump of rejected/s4-form-minus-banned-wrap-floor9.c (tmp/grind/func_80041188/s6/f9/*), tracing reg 86 through .rtl/.cse/.flow/.combine and comparing against the banked s5/va dumps.
- result: The condition is real but is the already-known cse.c make_regs_eqv canonicality rule, read in the other direction: on a copy `new = old`, NEW becomes the qty's canonical register iff regno_last_uid[new] outlives regno_last_uid[old]. In the floor-9 form the copy sits AFTER loop2's other pointer use, so 86 is canonical, cse1 has nothing to rewrite (.cse leaves insn 256 `86 = 87` and insn 263 `a1 = 86` untouched), flow counts both (5 refs / live 42) and combine substitutes 87 into 263 and deletes 256. The same rule pins the copy between the last-but-one and the last pointer use, so the def->use span is ~1 insn; any longer span steps over the second func_8004A348, giving the copy a second consumer and materialising it (+1 insn).
- verdict: KILLED

## [s6] The seats can be fixed with no reference lift by giving the pa4 carrier a hard-reg conflict with reg 22 ($s6), the way every allocno here conflicts with reg 16 ($s0) (s6 frontier #2).
- mechanism: global.c global_conflicts records a hard-reg conflict for every hard reg live across an allocno's range and find_reg excludes it in both passes.
- probe: Traced the reg-16 conflicts to their source in the .lreg dump (block-local, call-crossing quantities), read tools/gcc-2.7.2/local-alloc.c find_free_reg (~line 2249), and cross-checked $s0's usage in asm/funcs/func_80041188.s.
- result: The reg-16 conflict comes from loop1's `offset` temp - a block-1 local live across the first func_8004A348, hence needing a callee-saved register - and local-alloc's find_free_reg scans hard registers STRICTLY ASCENDING (MIPS defines no REG_ALLOC_ORDER; the only override, qty_phys_copy_sugg, is populated solely from hard-reg copies, i.e. call arguments/returns, all call-clobbered). Reaching hard reg 22 would need six further simultaneously-live call-crossing block-local quantities confined to loop2, and the first of them would take reg 16 and displace stptr2 from $s0. The target's own asm shows the identical $s0 loop1 offset temp (0x800411F8-0x80041238) and $s0 as stptr2 only from 0x800412A8, so the original had exactly one such conflict too.
- verdict: KILLED

## [s6] The banked tbl sym-K ref lift becomes usable if `i` (78) outranks the lifted tbl (79), which the never-swept C position of `i = 1` can supply.
- mechanism: pri = floor_log2(refs)*refs/live*10000 and seats are pure priority order (s6): i scores 2474 at 8/97 and 2500 at 8/96; tbl scores 2553 at 6/47 and 2500 at 6/48, so the relative def order of the two flips their ranking.
- probe: 30 variants (5 tbl-advance positions x 6 i-def positions), instrumented-cc1 allocno table for each, then sandbox --disable all on the 10 that seat correctly; plus 5 further variants (x1-x5) that move tbl's symbol initialiser out of the declaration into a statement after `i = 1;`.
- result: CONFIRMED as a seat fix: with `i = 1` placed after at least two preamble statements, i is 8 refs/live 96 = 2500.0 and tbl is 6/48 = 2500.0 - an exact tie broken our way on allocno number (78 < 79) - and EVERY callee-saved disposition equals target for the first time in this ledger (73 a1 s1, 74 a2 s2, 87 stptr s3, 90 stptr2 s0, 78 i s4, 79 tbl s5, 86 out2 s6, 77 pa4 s7, 75 a3 fp, 85 saved spilled). Sandbox 4 at 132 insns in all 10 spellings, with one identical residual: the preamble emits `sw s5,52 / lui s5 / addiu s5 / sw s4,48 / addiu s4,zero,1` where target emits the s4 pair first. That order is anti-correlated with the seats (see next hypothesis).
- verdict: CONFIRMED

## [s6] A loop-carried pseudo's reg_live_length can be moved by repositioning its uses inside the loop (the axis s5 recorded as 'tbl live scheduler-pinned to 45-47').
- mechanism: flow.c's reg_live_length counts every insn where the register is live.
- probe: 16 positions of loop1's `tbl++` (tmp/grind/func_80041188/s6/t/t01..t16), instrumented-cc1 allocno table for each.
- result: Bit-identical tables in all 16 (tbl 6 refs / live 47). A pseudo live across the loop back edge is live over the WHOLE body, so in-loop statement position cannot change its live length; only defs/uses outside the loop can. Separately measured: the live length moves by exactly +/-1 with the relative ORDER of the two preamble defs (i-first => i 97 / tbl 47 in every a*i0 and x1-x5 variant; tbl-first => 96 / 48 in every a0i2..a3i5 variant), and sched1 emits the two defs in that same order (equal INSN_PRIORITY, LUID tie-break). The 'advance position' axis is retired.
- verdict: KILLED

## s7 (rederive, 2026-08-23)

### KILLED
- **H-s7-1 — "the two `a4 + 0x20` values in target are one variable, and loop.c/LICM
  placed the second definition."** KILLED. Removing every out-pointer local and
  writing `a4 + 8` inline at all four call sites makes the expression materialise
  INSIDE both loops, never in a preheader (sandbox 63). There are no loop notes on
  these goto-loops, so LICM never runs on them. Both `addiu ...,$s7,0x20` insns in
  target are source-level statements, and since GCC 2.7.2 cannot split a live range,
  the two registers ($s6 and $s3) mean two distinct source locals.
  Evidence E-s7-1/E-s7-2. Artifact `rejected/inline-a4plus8-no-licm-hoist.c`.
- **H-s7-2 — "declaration/statement order inside block 0 can lengthen out2's
  reg_live_length enough to move its allocno priority."** KILLED. Seven block-0
  statement permutations x two out2 spellings collapse into exactly three score
  classes with identical `.greg` dispositions inside each class; moving out2's
  definition to the first statement of the body bought +1 insn of live length
  (42 -> 43), not the ~8 predicted. Evidence E-s7-8.
  Artifact `rejected/block0-statement-order-live-length-dead.c`.
- **H-s7-3 — "a split-init (`out2 = a4; out2 += 8;`) buys out2 a flow-counted
  reference."** KILLED again, now in the new chassis and with the mechanism named:
  the definition lives in block 0, which is a cse1 extended basic block where out2's
  value is known, so cse1 folds the pair and flow deletes the remnant UNCOUNTED.
  All four spellings (on out2, on out3, `+4/+4`, and from `a4` rather than `pa4`)
  scored an unchanged 15. Evidence E-s7-7.
- **H-s7-4 — "an out2-derived expression inside loop1 is a byte-free reference."**
  KILLED. loop1 starts a fresh cse1 ebb (2 preds) where out2 is only a live-in
  value, so `out2 - 8` cannot be folded back to a4 and materialises
  `addiu $a1,$s5,-32` against target's `addu $a1,$s7,$zero`. It also overshoots:
  4 refs / live 42 = 1904.8 > tbl's 1702.1, so out2 steals $s5.
  Artifact `rejected/out2-minus-8-loop1-ref-not-byte-free.c`.
- **H-s7-5 — "out2 might reach target's seat with only 3 references."** KILLED by
  arithmetic, not by search: a4 cannot drop below 6 materialised references (two
  call arguments per loop plus the two out-pointer definitions) at live ~95, so its
  priority floor is 1263.2, while out2 with 3 references and live <= 47 tops out at
  714.3. A 4th reference on out2 is NECESSARY. Evidence E-s7-6.

### CONFIRMED
- **H-s7-6 — the seat order is decided purely by allocno priority, and target's
  order is `tbl > out2 > a4 > a3`.** CONFIRMED end-to-end: with
  `out3 = out2;` last in the between-loops block, `.lreg` gives tbl 4/47 and out2
  4/47 — an EXACT tie at 1702.1 — broken by allocno number (79 < 86) in tbl's
  favour, and `.greg` `;; Register dispositions:` shows 79 in 21 ($s5), 86 in 22
  ($s6), 77 in 23 ($s7), 75 in 30 ($fp), 87 in 19 ($s3). All five seats are target.
  This confirms the s5/s6 banked window `pri(out2) in (1473.7, 1702.1)` and pins
  the answer to its closed upper endpoint. Evidence E-s7-4.
- **H-s7-7 — the whole match is reachable without any sanctioned-exception
  construct.** CONFIRMED to distance 1: candidate.c is ordinary C throughout
  (two separate out-pointer locals, two separate loop walkers, every local written
  once and read for its real value) and scores 1 with 132/132 insns. The
  `/* FAKE */` variable-reuse annotation the s3–s6 chassis needed is GONE.

## s7 frontier (for s8)
1. **A byte-free 4th reference to out2 in the BETWEEN-LOOPS block closes the
   function outright.** Everything else in candidate.c is already target: all
   five contested seats, all 132 instructions, the frame size. The single residual
   is slot 72, `move $s3,$s6` vs `addiu $s3,$s7,0x20`, and it exists only because
   the 4th reference is currently spent as the copy `out3 = out2;`. If out3 is
   defined from a4 instead, out2 falls to 3 refs / live 42 and the seats permute.
   *Mechanism:* flow.c fixes `reg_n_refs` and `reg_live_length` BEFORE combine
   runs, so any out2 mention in that block that combine subsequently deletes is a
   free +1 reference and a free +5 live length. combine only deletes an insn whose
   destination is used exactly once and dies in the combining insn, and only within
   a single basic block — so the consumer must be one of the six insns target
   actually emits in the between-loops block (`addiu $s1,$s1,0x6C`,
   `addiu $s2,$s2,0x6C`, `addiu $s4,$zero,0x12`, `lw $t0,0x18($sp)`,
   `addiu $s3,$s7,0x20`, `addiu $s0,$t0,0x750`).
   *Next probe:* enumerate C spellings in which one of those six values is written
   as a single-use function of out2 that combine can fold back to its target form —
   the most promising is `out3`, written so that the copy from out2 has exactly one
   consumer at combine time. Dump `.combine` for each and grep for the deleted-insn
   record before trusting the score.
2. **Attack the tie from tbl's side instead: shrink tbl's reg_live_length to <= 42
   so that an out2 with 4 refs / live 42 (a reference inside loop1) still loses the
   tie to tbl.** tbl measures 4 refs / live 47, of which 41 is loop1's block and 6
   is a block-0 lead-in from its `lui/addiu` symbol definition. If that lead-in
   shrinks to <= 1, tbl reaches 4/42 = 1904.8 and ties-or-beats an in-loop1 out2 at
   the same 1904.8 (allocno 79 < 86 wins), which would let `out3` be defined from a4
   and emit `addiu $s3,$s7,0x20` directly.
   *Mechanism:* the 6-insn lead-in is the distance, in the pre-sched insn stream
   flow sees, between tbl's definition and loop1's label. s7 measured that moving
   the source statement does NOT move it (E-s7-8), so something else pins it —
   read `.rtl`/`.loop`/`.cse2` for func_80041188 and find which pass places the
   `lui/addiu %hi/%lo` pair, rather than guessing.
   *Next probe:* dump `.rtl` and `.cse2`, locate tbl's definition insn and count
   insns to the loop1 `code_label`; then look for a source form whose tbl
   definition is emitted later in that stream (e.g. tbl reached through a different
   expression, or the D_80094CFC address consumed differently). Confirm with
   `.lreg` `Register NN used 4 times across NN insns` before scoring.
3. **The `pa4 = a4` param-local alias is load-bearing and should be understood,
   not just kept.** Dropping it costs 12 points (1 -> 13) with everything else
   identical. *Mechanism:* it changes which pseudo carries a4 and hence a4's
   `reg_n_refs` (6 with the alias, and the parameter's own incoming copy is
   accounted separately), which is one of the four numbers in the priority table.
   *Next probe:* diff `.lreg` for candidate.c against `rejected/no-pa4-param-alias.c`
   and record exactly which pseudo's refs/live changed. If the alias turns out to be
   equivalent to some natural spelling (e.g. the original taking the pointer through
   a typed local such as `MATRIX *m = (MATRIX *)a4;`), prefer that spelling — it is
   the same codegen with a semantics a reviewer can read.

## [s7] The two `a4 + 0x20` values in target are one C variable, and loop.c/LICM placed the second definition in the between-loops block.
- mechanism: If loop.c recognised loop1 and loop2 it would hoist the loop-invariant `a4 + 8` into each loop's preheader, producing the two `addiu ...,$s7,0x20` insns from a single source expression.
- probe: Deleted every out-pointer local and wrote `a4 + 8` inline at all four call sites; measured sandbox and disassembled the .o against target.
- result: sandbox 63, 132 insns. The expression materialises INSIDE loop1 (`addiu $s1,$s8,32`) and INSIDE loop2 (`addiu $s0,$s8,32`) - never in a preheader or the preamble. These goto-loops carry no loop notes, so LICM never runs. Combined with the fact that GCC 2.7.2 has no live-range splitting, target's two DIFFERENT registers ($s6 across loop1, $s3 across loop2) prove two distinct source-level locals.
- verdict: KILLED

## [s7] Declaration/statement order inside block 0 can lengthen out2's reg_live_length enough to move its allocno priority into target's window.
- mechanism: reg_live_length is measured by flow.c on the pre-scheduling insn stream, so a definition placed earlier in block 0 should be live across every block-0 insn that follows it.
- probe: Swept 7 block-0 statement permutations (P0..P6) crossed with two out2 spellings = 14 builds; read reg_n_refs/reg_live_length from .lreg and the seat assignment from the .greg `;; Register dispositions:` line for the extremes.
- result: Exactly three score classes - 15, 10, 18 - with byte-identical .greg dispositions inside each class. Moving out2's definition to the very first statement of the body raised its live length from 42 to only 43 (+1), not the ~8 a source-order model predicts. Block-0 statement ordering is a dead lever for this function.
- verdict: KILLED

## [s7] A split-init (`out2 = a4; out2 += 8;`) buys out2 a flow-counted reference.
- mechanism: Two RTL insns for one value should give the pseudo an extra reference at flow time, with combine merging them back so no byte is spent.
- probe: Four spellings measured: split-init on out2, split-init on out3, a `+4/+4` two-step, and initialising out2 from the raw `a4` parameter instead of the `pa4` alias.
- result: All four scored an unchanged 15. Mechanism now named: out2's definition sits in block 0, which is a cse1 extended basic block in which out2's value is known, so cse1 folds the pair pre-flow and flow deletes the remnant UNCOUNTED. This re-kills the family in the new chassis, matching the s6 finding.
- verdict: KILLED

## [s7] An out2-derived expression inside loop1 (e.g. spelling loop1's first call argument `out2 - 8` instead of `a4`) is a byte-free 4th reference.
- mechanism: cse1 knows out2 == a4 + 0x20, so `out2 - 8` should fold back to a4 and leave only the extra reference behind.
- probe: Built that spelling, measured sandbox, disassembled, and read .lreg.
- result: sandbox 10 - better than 15 but not free. loop1 begins a NEW cse1 ebb (2 predecessors) where out2 is only a live-in value, so cse1 cannot fold and the expression materialises as `addiu $a1,$s5,-32` against target's `addu $a1,$s7,$zero`. It also overshoots the window: 4 refs / live 42 = 1904.8 > tbl's 1702.1, so out2 steals $s5. Generalises with E-s7-7: block-0 out2 expressions are folded away uncounted, non-block-0 ones cost an instruction.
- verdict: KILLED

## [s7] out2 could reach target's $s6 seat with only 3 references, if a4's or a3's numbers were pushed instead.
- mechanism: pri = floor_log2(refs)*refs/live*10000, so lowering a4 or a3 would be an alternative to raising out2.
- probe: Closed-form over the measured .lreg numbers rather than by search: a4 carries at least 6 materialised references (two call arguments per loop plus both out-pointer definitions) at live ~95, and a3 is a parameter with 4 references (3 call arguments) at live 99 that cannot be reduced.
- result: a4's priority floor is 1263.2 and a3's is fixed at 808.1, while out2 with 3 references and live <= 47 tops out at 714.3. No 3-reference spelling of out2 can outrank a4. A 4th reference on out2 is NECESSARY, not one option among several.
- verdict: KILLED

## [s7] The seat order is decided purely by allocno priority, and target requires `tbl > out2 > a4 > a3`; giving out2 a 4th reference by reading it in the between-loops block satisfies it.
- mechanism: global.c ranks allocnos by pri = floor_log2(reg_n_refs)*reg_n_refs/reg_live_length*10000 and find_free_reg scans ascending, so the four contested pseudos must be processed in that order to receive $s5, $s6, $s7, $fp.
- probe: Placed `out3 = out2;` as the LAST statement of the between-loops block, swept its position across all 5 slots in that block, and read .lreg + the .greg `;; Register dispositions:` line.
- result: CONFIRMED. Last slot scores 1; slots 3/2/1/0 score 10/11/12/12. At the last slot .lreg gives tbl 4 refs/47 and out2 4 refs/47 - an EXACT tie at 1702.1 - broken by allocno number (79 < 86) in tbl's favour, and .greg shows 79 in 21 ($s5), 86 in 22 ($s6), 77 in 23 ($s7), 75 in 30 ($fp), 87 in 19 ($s3): all five seats target. This confirms the s5/s6 banked window pri(out2) in (1473.7, 1702.1) and pins the answer to its closed upper endpoint.
- verdict: CONFIRMED

## [s7] Distance 1 on this function is reachable without any sanctioned-exception construct - no /* FAKE */, no variable reuse, no do-while(0) wrap.
- mechanism: The s3-s6 chassis needed a FAKE only because it forced one pseudo to serve as both loop1's walker and loop2's output pointer; two separate locals remove the need entirely.
- probe: Built the two-locals form with separate `stptr`/`stptr2` walkers and separate `out2`/`out3` output pointers, every local written once and read for its real value; measured sandbox and disassembled all 132 insns against target.
- result: CONFIRMED. sandbox 1, 132/132 insns, frame 72 == target 0x48. Every instruction matches target 1:1 except slot 72. candidate.c now carries no register pin, no inline asm, no volatile, no dead local, no annotation.
- verdict: CONFIRMED

## s8 (rederive, 2026-08-23)

Chassis re-verified at dispatch: candidate.c applied to src/text1a_pre.c scores
`sandbox --disable all` = **1**, 132/132 insns. Every number below was measured
THIS session against that chassis, using a single-function extraction TU
(`tmp/grind/func_80041188/s8/fd.c`) that reproduces the full-TU `.lreg` table
pseudo-for-pseudo (verified identical to the `text1a_pre.c` dump), so each
variant costs one cc1 run instead of a full sandbox build.

### KILLED
- **H-s8-1 - "tbl's 6-insn block-0 lead-in (reg_live_length 47 vs loop1's 41)
  can be shortened by moving `tbl = D_80094CFC;` later in block 0"
  (s7 frontier #2).** KILLED. Moving the assignment out of the declaration and
  making it the LAST statement of block 0 produced a `.lreg` table
  BIT-IDENTICAL to the baseline in every pseudo (tbl still 4 refs / live 47) and
  the identical sandbox 1. Block-0 source position of tbl's definition is inert.
  Artifact `rejected/tbl-assign-position-inert-live47.c`.
- **H-s8-2 - "a dead store of out2 in the between-loops block buys a
  flow-counted 4th reference."** KILLED with the mechanism named: flow.c counts
  `reg_n_refs[regno] += loop_depth` only on the *needed* branch of
  `propagate_block` (flow.c:2081), so a SET whose destination is dead is deleted
  WITHOUT being counted. Two spellings measured (`out3 = out2;` immediately
  overwritten by `out3 = pa4+0x20;`, and a store into a separate never-read
  local): out2 stays 3 refs / live 42 in both, byte-identical to the plain
  `out3 = pa4+0x20` form. Artifact `rejected/dead-store-out2-uncounted-by-flow.c`.
- **H-s8-3 - "loop1 and/or loop2 spelled as real `do { } while` loops (which
  emit NOTE_INSN_LOOP_BEG/END and therefore double every in-loop reference via
  flow.c's loop_depth weighting) re-open the seating in the s7 TWO-LOCALS
  chassis."** (s4 measured this in the old single-out2 chassis; the brief's
  chassis-relative rule required re-measurement.) KILLED again. loop1-real +
  `out3 = out2` gives the CORRECT priority ORDER for the first time
  (tbl 7/47=2978 > out2 6/46=2608 > pa4 8/94=2553 > a3 5/99=1010) but scores 5,
  because loop.c rewrites loop1's induction variables (stptr becomes a new
  pseudo 125 at 9 refs / live 40) and changes 5 in-loop instructions.
  loop1-real + `out3 = pa4+0x20` scores 13 and has the WRONG order
  (pa4 9/94=2872 outranks out2 5/41=2439): `out3 = pa4+0x20` inherently hands pa4
  another reference, so that chassis can never put out2 above pa4.
  Both-real was measured at s4 as +3 insns (loop.c preheader) and was not re-run.
  Artifacts `rejected/loop1-real-do-while-newchassis-score5.c`,
  `rejected/loop1-real-out3-from-pa4-pa4-outranks-out2.c`.
- **H-s8-4 - "s7 frontier #1: some out2 reference in the between-loops block is
  deleted by combine and is therefore byte-free."** KILLED as a CLASS, by
  mechanism plus a positive control. Positive control (`H_combine_ref_premise`):
  a separated staging pair `tmp = out2 + 0x10; out3 = tmp - 0x10;` DOES survive
  cse1/cse2, IS counted by flow (out2 back to 4 refs / live 47, table identical
  to baseline) and IS deleted by combine - so the counting premise is REAL. But
  combine only deletes an insn by substituting its value into the surviving
  consumer, and LOG_LINKS are intra-block, so the consumer must be one of the
  six insns target emits between the loops
  (`addiu $s1,$s1,0x6C`, `addiu $s2,$s2,0x6C`, `addiu $s4,$zero,0x12`,
  `lw $t0,0x18($sp)`, `addiu $s3,$s7,0x20`, `addiu $s0,$t0,0x750`). NONE of the
  six reads $s6 or any value derived from out2; the only one that could consume
  out2 is out3's definition, and consuming it produces `move $s3,$s6` - the
  residual itself. The only escape would be an out2 term that cancels
  (`out2 - out2`), i.e. forbidden opaque arithmetic. Therefore no byte-free out2
  reference exists in the between-loops block, and s7 frontier #1 is closed.

### CONFIRMED
- **H-s8-5 - a between-loops RESTORE of out2 from pa4, SEPARATED from the
  `out3 = out2` copy by one intervening statement, makes GCC emit target's
  residual insn `addiu $s3,$s7,0x20` while keeping out2's between-block
  references flow-counted.** CONFIRMED. Form (`I2`):
  `out2 = (s32 *)((u8 *)pa4 + 0x20); stptr2 = saved + 0x750; out3 = out2;`
  Measured: cc1 emits `addu $19,$23,32` (= `addiu $s3,$s7,0x20`) at the
  between-loops slot - the FIRST time in this ledger that target's slot-72 form
  has been produced - 132 insns, sandbox **9**. The residual 9 is now PURELY a
  seat swap: out2 reaches 5 refs / live 42 = 2380.9 and outranks tbl
  (4/47 = 1702.1), so out2 takes $s5 and tbl takes $s6 (cc1 emits
  `addu $21,$23,32` for out2's block-0 definition instead of `addu $22,...`).
  SEPARATION IS LOAD-BEARING and the mechanism is cse1's last-set propagation:
  with the restore placed IMMEDIATELY before the copy, cse1 substitutes the
  SET_SRC into the copy, the restore becomes dead and flow deletes it uncounted
  - out2 falls back to 3 refs / live 42 and the form degenerates into the plain
  `out3 = pa4 + 0x20` variant (identical table, s7's score-15 class).
  Artifacts `rejected/i2-separated-restore-emits-target-insn-seatswap9.c`,
  `rejected/adjacent-restore-folded-by-cse1-equals-plain-pa4.c`.

### The s8 arithmetic (the whole remaining problem, in closed form)
In the I2 chassis every pseudo except tbl is already where target wants it. The
measured priorities are
    88 stptr   7/41 = 3414      91 stptr2 6/48 = 2500     78 i    8/97 = 2474
    86 out2    5/42 = 2380      79 tbl    4/47 = 1702     77 pa4  7/95 = 1473
    75 a3      4/99 =  808      87 out3   3/47 =  638     85 saved 2/47 = 425
and the ONLY unsatisfied constraint is `pri(tbl)` needing to land in
`[2380.9, 2474.2]` (ties are fine at both ends: tbl's allocno 79 beats out2's 86
and loses to i's 78, which is exactly what target needs). Enumerating
`floor_log2(n)*n/L*10000` for tbl:
  * 4 refs -> L in [32.3, 33.6)  - impossible, loop1's block alone is 41 insns
  * 5 refs -> L in [40.4, 42.0)  - needs live SHORTENED to 41 AND +1 ref
  * 6 refs -> L in [48.5, 50.4)  - i.e. **6 refs at live 49 or 50**
  * 7 refs -> L in [56.6, 58.8)
The 6-ref lift is already banked (s4's sym-K split-init chain, tbl 4 -> 6 refs,
byte-neutral, `rejected/tbl-symK-splitinit-refs6-steals-s4.c`) but it pins tbl at
live 45-47 = 2553-2666, ABOVE i, so it steals $s4. The remaining need is
therefore +2..+3 on tbl's reg_live_length. Because i is live over the whole
function while tbl is live over 47 insns, ANY insn added to block 0 after tbl's
definition raises both: with N insns added, i = 24/(97+N) and tbl = 12/(47+N),
and tbl <= i requires 97+N <= 2*(47+N), i.e. **N >= 3**. At N = 3 both sit at
exactly 2400.0 and the tie breaks on allocno 78 < 79 - i keeps $s4, tbl takes
$s5 - while out2 at 5/42 = 2380 (or 5/45 = 2222 if the added insns follow out2's
definition) stays below. That is also the i-FIRST preamble order, which is the
order target emits, so it does NOT reproduce the emission-order residual that
rejected s6's all-target-seats form
(`rejected/tbl-symK-ilate-alltarget-seats-emission-swapped.c`, sandbox 4).
The open question is whether three byte-free block-0 insns exist: H-s8-5's
separated-staging mechanism produces exactly that (survives cse, counted by flow,
deleted by combine), but H-s7-3 measured that split-inits inside block 0 are
folded by cse1 pre-flow - the difference between the two is the intervening
statement, which has never been tried in block 0.

## s8 frontier (for s9)
1. **Does the H-s8-5 SEPARATED-staging mechanism work inside block 0?** H-s7-3
   killed block-0 split-init because cse1 folds it; H-s8-5 showed the fold is
   cse1's *last-set* propagation and that it disappears when one statement
   intervenes.
   *Mechanism:* cse1 substitutes a SET_SRC into the immediately following insn;
   with an intervening insn the lookup returns the register instead, both insns
   survive to flow (and are counted), and combine merges them afterwards.
   *Next probe:* on the I2 chassis, write out2's (or saved's, or stptr's) block-0
   definition as a SEPARATED two-step placed AFTER tbl's definition, and read
   tbl's `Register 79 used N times across M insns` line in `.lreg`. Each
   surviving staging insn should give tbl +1 live. Target state: tbl live 50 with
   the s4 sym-K 6-ref lift and i live 100, both at 2400.0. Confirm in `.combine`
   that the staging insn is deleted before trusting the sandbox score.
2. **Shorten tbl to live 41 and add one reference (the 5-ref window).** tbl at
   5 refs / live 41 = 2439 also sits in the window and needs no i-side change.
   Live 41 means tbl's whole live range is loop1's block, i.e. its definition
   would have to be the last insn before the loop1 label - H-s8-1 proved source
   position cannot do that, so this needs a different definition SHAPE (tbl
   reached through a value that is itself computed last in block 0).
   *Next probe:* dump `.rtl` and `.flow` for the I2 chassis, locate reg 79's
   definition insn and count the insns between it and loop1's code_label, to
   find out WHAT the six trailing block-0 insns are and whether any of them can
   be moved above tbl's definition.
3. **Watch the construct budget.** The I2 restore is a same-value re-store of a
   LOCAL (`dead-store-fake-exception` family, FAKE + lever exhaustion required),
   and the s4 sym-K chain plus any block-0 separated staging are
   combine-foldable chain-extenders (the F1 family, FAKE-annotated last resort).
   A form that stacks all three will very likely draw a layer-1 FAIL on
   aggregate. Before building it, look for a spelling of the between-loops
   restore that a reader can justify semantically - e.g. the two matrices
   spelled through a real `MATRIX *` typed local so that `m1 = &m[1]`
   re-appearing before the second loop reads as ordinary scoping rather than as
   a re-store. (a4's two 0x20-byte objects are PsyQ `MATRIX`-sized, which is the
   most likely original spelling and has never been tried.)

## [s8] tbl's 6-insn block-0 lead-in (reg_live_length 47 vs loop1's 41 insns) can be shortened by moving `tbl = D_80094CFC;` later in block 0 (s7 frontier #2).
- mechanism: flow.c's reg_live_length counts every insn in which a pseudo is live; if tbl's definition were the last block-0 insn its lead-in would be 1 and its live length would fall to 42, letting it tie an out2 that carries its 4th reference inside loop1.
- probe: Moved the initialiser out of the declaration and made `tbl = D_80094CFC;` the LAST statement of block 0; read the full .lreg allocation table and ran sandbox --disable all.
- result: The .lreg table is BIT-IDENTICAL to the baseline in every pseudo (tbl still 4 refs / live 47; out2 4/47; pa4 6/95) and the sandbox score is unchanged at 1. Combined with s6's 16-position tbl++ sweep, tbl's reg_live_length is not reachable by source statement position at all.
- verdict: KILLED

## [s8] A dead store of out2 in the between-loops block buys a flow-counted 4th reference (out2's live length and refs are fixed before combine, so a store that is later deleted should still be counted).
- mechanism: reg_n_refs and reg_live_length are set by flow_analysis (toplev.c:2983) before combine (toplev.c:3004), so anything deleted after flow is free.
- probe: Two spellings on the `out3 = pa4 + 0x20` chassis: (a) `out3 = out2;` immediately overwritten by `out3 = (s32*)((u8*)pa4+0x20);`, (b) a store into a separate never-read local `dead`. Read .lreg for both.
- result: out2 stays at 3 refs / live 42 in both, and both are byte-identical to the plain form. Mechanism read out of the compiler: flow.c:2081 increments reg_n_refs only on the branch where the SET is needed, so propagate_block deletes a dead set WITHOUT counting it. Dead stores are inert here in both directions.
- verdict: KILLED

## [s8] loop1 and/or loop2 spelled as real `do { } while` loops re-open the seating in the s7 two-locals chassis (s4 measured this only in the retired single-out2 chassis, and the brief's chassis-relative rule requires re-measurement).
- mechanism: flow.c weights reg_n_refs by loop_depth, which is driven by NOTE_INSN_LOOP_BEG/END; only a real C loop emits those notes, so every in-loop reference counts twice and the whole priority table rescales.
- probe: Four variants measured (.lreg + sandbox): loop1-real with `out3 = out2`; loop1-real with `out3 = pa4+0x20`; both loops real with `out3 = pa4+0x20`; and the goto-loop control with `out3 = pa4+0x20`.
- result: loop1-real + `out3 = out2` produces the CORRECT priority order for the first time in the ledger (tbl 7/47=2978 > out2 6/46=2608 > pa4 8/94=2553 > a3 5/99=1010) but scores 5, because loop.c now runs on the note-marked loop and rewrites the induction variables - loop1's walker becomes a NEW pseudo 125 at 9 refs / live 40 and five in-loop instructions change. loop1-real + `out3 = pa4+0x20` scores 13 with the WRONG order, since that spelling hands pa4 a 9th reference (2872) and no 5-reference out2 at live >= 41 can outrank it. Real loops are dead in this chassis too.
- verdict: KILLED

## [s8] s7 frontier #1: some out2 reference in the BETWEEN-LOOPS block is deleted by combine and is therefore a byte-free 4th reference, letting out3 be defined from pa4 while out2 keeps 4 refs / live 47.
- mechanism: flow fixes reg_n_refs and reg_live_length before combine runs, so any out2 mention combine subsequently deletes is free; combine deletes an insn whose destination is used exactly once and dies in the combining insn, within one basic block.
- probe: Positive control first: `tmp = (s32*)((u8*)out2 + 0x10); out3 = (s32*)((u8*)tmp - 0x10);` in the between-loops block, .lreg + sandbox. Then enumerated the consumers combine could possibly have, against the six insns target actually emits in that block (asm/funcs/func_80041188.s:70-75).
- result: The counting premise is CONFIRMED - the staged pair restores out2 to exactly 4 refs / live 47 (the baseline table) at 132 insns / sandbox 1, i.e. flow counted it and combine folded it away. But the CLASS is dead: combine's LOG_LINKS are intra-block and it deletes an insn only by substituting its value into the surviving consumer, and none of the six surviving between-block insns (`addiu $s1,$s1,0x6C`, `addiu $s2,$s2,0x6C`, `addiu $s4,$zero,0x12`, `lw $t0,0x18($sp)`, `addiu $s3,$s7,0x20`, `addiu $s0,$t0,0x750`) reads $s6 or any out2-derived value. The only possible consumer is out3's own definition, which then emits `move $s3,$s6` - the residual itself. The only escape is a cancelling term (`out2 - out2`), i.e. forbidden opaque arithmetic.
- verdict: KILLED

## [s8] A between-loops RESTORE of out2 from pa4, SEPARATED from the `out3 = out2` copy by one intervening statement, makes GCC emit target's residual insn `addiu $s3,$s7,0x20` while keeping out2's between-block references flow-counted.
- mechanism: cse1 propagates a SET_SRC into the immediately following insn (last-set substitution) but falls back to the register when a statement intervenes; both insns then survive to flow and are counted, and combine merges `out2 = pa4+0x20` into `out3 = out2` afterwards - deleting the restore and leaving exactly one insn, `addiu $s3,$s7,0x20`.
- probe: Built `out2 = (s32*)((u8*)pa4 + 0x20); stptr2 = saved + 0x750; out3 = out2;` (variant I2) and the adjacent control; read .lreg, read the emitted cc1 asm for the between-loops block, and ran sandbox --disable all on both.
- result: CONFIRMED. I2 emits `addu $19,$23,32` (= `addiu $s3,$s7,0x20`) at the between-loops slot - the first time in this ledger that target's slot-72 form has been produced - at 132 insns and sandbox 9. The nine residual diffs are purely the $s5/$s6 swap: out2 reaches 5 refs / live 42 = 2380.9 and now outranks tbl at 4/47 = 1702.1. The adjacent control degenerates to out2 3 refs / live 42, i.e. to the plain `out3 = pa4+0x20` score-15 class, which also re-explains H-s7-3: block-0 split-inits die from ADJACENCY, not from being in block 0.
- verdict: CONFIRMED

## [s9] The H-s8-5 SEPARATED-staging mechanism works inside block 0 too, and three such staging insns after tbl's definition raise tbl's reg_live_length from 47 to 50 byte-free (s8 frontier #1).
- mechanism: cse1 substitutes a SET_SRC into the immediately following insn but not across an intervening statement, so separated two-step definitions survive cse1/cse2 into flow (where reg_n_refs and reg_live_length are fixed) and are merged back by combine afterwards, costing no bytes; each surviving insn after tbl's definition adds +1 to tbl's live length.
- probe: Built the extraction-TU harness with a mechanical seat checker (tmp/grind/func_80041188/s9/run.sh reads `;; Register dispositions:` from .greg) and swept nineteen block-0 variants on the I2 chassis: the sym-K advance in five slots, the stptr split-init in three placements, the saved split-init alone and combined with stptr's, a `base` two-step, a fresh-intermediate copy, and three placements of `i = 1`. Read `Register 79 used N times across M insns` for each.
- result: HALF-CONFIRMED, and the useful half is the negative one. Separated staging DOES survive in block 0 - but only in one spelling (a self-update of the same pseudo with a register-valued first step), and tbl's reg_live_length NEVER reaches 48 from any of them: every variant reads 45 or 47. The predicted "+1 live per staging insn" is false. The one variant that did reach tbl 6/48 = 2500 with i 8/96 = 2500 (the exact winning tie) did so by lifting `saved` from 2 refs to 5, which makes saved a tenth claimant on nine callee-saved seats and spills a3 - fatal, because target spills saved. Two further spellings were killed with named mechanisms: a fresh intermediate is copy-propagated by cse1 and never counted, and a constant two-step is constant-folded.
- verdict: KILLED (as stated: three staging insns are not obtainable; the requirement is now known to be exactly ONE surviving insn from a pseudo that already holds a seat).

## [s9] tbl reaches the priority window from the 5-reference side at live 41, i.e. with its definition as the last insn before loop1 (s8 frontier #2).
- mechanism: reg_live_length counts the insns in which a pseudo is live; H-s8-1 proved the SOURCE position of tbl's assignment cannot move that count, so what pins it is the six block-0 insns following the definition in the pre-flow RTL stream, and moving those above the definition is a different lever.
- probe: Instead of the .rtl/.flow insn census the s8 frontier proposed, swept the position of the sym-K ADVANCE insn (which is what actually starts tbl's counted live range once the split-init exists) across every slot of block 0, and separately swept the position of `i = 1` across three slots that keep it before tbl.
- result: KILLED, and with a stronger statement than the frontier asked for. tbl's live length is not merely position-invariant upward - it has a hard CEILING of 47 (= loop1's 41 insns + a fixed 6-insn block-0 tail). The advance's position can LOWER it to 45 (advance last) but nothing raises it. `i = 1`'s position inside block 0 is completely inert; three placements give bit-identical .lreg tables. Therefore the 5-refs/live-41 window is unreachable in the opposite direction as well: tbl's live length is a one-sided quantity in this chassis.
- verdict: KILLED

## [s9] Target's preamble emission order and tbl's winning live length can be had simultaneously, because sched1 runs after flow and could be steered independently.
- mechanism: flow_analysis (toplev.c:2983) fixes reg_live_length from the RTL insn order; sched1 (toplev.c:3033) decides emission order from INSN_PRIORITY, then a dependence class relative to last_scheduled_insn, then INSN_LUID. If the two defs could be separated on priority or class, RTL order (hence live lengths) and emission order would decouple.
- probe: Read sched.c rank_for_schedule end-to-end and the function's own .sched dump for the priorities and ready-list trace of the two definition insns.
- result: KILLED for every lever available from C. Both defs are leaves - `.sched` reports `priority = 1, ref_count = 0` for each, because neither has an in-block consumer - so the priority test is a tie; neither can take a dependence class other than 3 (i's def is a `li` with no inputs, so it cannot be data-dependent on anything, and nothing else writes either pseudo in block 0), so the class test is a tie; and the final `INSN_LUID (y) - INSN_LUID (x)` fallback with backward block scheduling reproduces RTL order exactly. Raising tbl's def priority would need an in-block consumer that survives combine into sched1, which by definition materialises a byte. The anti-correlation is structural, not incidental.
- verdict: KILLED

## [s9] i's live length can be shortened enough for the i-FIRST order to seat tbl, by exploiting the dead hole in i's live range between loop1's exit test and `i = 0x12`.
- mechanism: flow.c:1685 counts only insns in which the pseudo is live, and i is dead across the leading part of the between-loops block; moving its re-definition later grows that hole and shortens reg_live_length[i].
- probe: Four-position sweep of `i = 0x12;` inside the 6-statement between-loops block on the I2+symK chassis, plus a control that ADDS a surviving insn to the hole (a separated self-update of stptr2) to see whether the hole can also be grown from the left.
- result: CONFIRMED as a lever, KILLED as a solution. The sweep works and is new to the ledger: i live 97 -> 96 -> 95 -> 95 as the re-definition moves to the 4th, 5th and 6th statement. But 95 (pri 2526) is the floor, and tbl's ceiling of 6/47 = 2553 still outranks it by 27 points. The left-edge control confirms the hole is one-sided: adding a demonstrably-surviving insn before `i = 0x12` (stptr2 6 -> 8 refs, a1/a2 live 99 -> 100) leaves i at 8/95 unchanged, because those insns sit where i is already dead.
- verdict: CONFIRMED (lever) / KILLED (as a standalone solution)

## [s9] i can be given a ninth flow-counted reference, which would put it at 27/95 = 2842 and close the i-FIRST order outright without touching any other pseudo.
- mechanism: reg_n_refs is fixed by flow before combine runs, so a reference in an insn that combine later deletes is free; floor_log2(9)*9 = 27 against floor_log2(8)*8 = 24 is a 12.5% jump in the numerator with no live-length cost if the added insn sits in i's dead hole.
- probe: `i = 0x10; a2 += 0x6C; i += 2;` in the between-loops block - a separated self-update, the shape that survives for register-valued pseudos - measured against `i = 0x12;`.
- result: KILLED for the constant spelling. cse1 constant-folds the pair before flow sees it and i stays at 8 refs / live 97, bit-identical to the plain form. This matches the third case of the refined cse1 survival rule (E-s9-9): self-updates survive only when the first step is register-valued. A register-valued self-update of i has not been tried and is the single most valuable open probe.
- verdict: KILLED (constant spelling only; the family remains open)

## s9 frontier (for s10)
1. **A register-valued ninth reference for `i`.** This is the highest-value open
   probe in the ledger: nine flow-counted references put i at 27/95 = 2842,
   clear of tbl's 2553 ceiling, in the i-FIRST definition order that already
   produces target's preamble emission - and it needs no change to tbl, out2,
   stptr, stptr2, saved, pa4 or a3.
   *Mechanism:* E-s9-9's survival rule - a self-update `i = i OP x` with a
   REGISTER-valued operand and an intervening statement survives cse1 into flow
   (constants are folded, fresh intermediates are copy-propagated), is counted at
   flow.c:2081, and is merged back by combine before any byte is emitted.
   *Next probe:* find a register-valued expression for i's re-definition in the
   between-loops block that is semantically honest. Note loop1 exits with i
   already equal to 0x12, so `i = 0x12;` is arithmetically a no-op there; look
   for a spelling that derives loop2's starting index from a value that is live
   at that point (a1/a2's advance, stptr, saved) rather than from a literal, and
   measure `Register 78 used N times` in .lreg. Also try the self-update inside
   loop1 (`i++` split with a register operand), which adds +1 to tbl's live as
   well and would land the 2500/2500 tie from the other side.
2. **A surviving block-0 insn whose donor already holds a callee-saved seat.**
   E-s9-7/8 reduce the i-first order to exactly one requirement: one insn that
   survives cse1 into flow, sits after tbl's definition in block 0, and does not
   create a tenth claimant on the nine callee-saved seats. `saved` is the wrong
   donor (it is spilled in target). `stptr` (7/41 = 3414, comfortably top) is the
   right shape of donor but its separated split measured +1 ref / +3 live with no
   added insn in three placements.
   *Mechanism:* same cse1 self-update rule; the donor's priority must stay in its
   target band so the seat order is untouched.
   *Next probe:* enumerate self-update spellings of `stptr` and of `pa4` in block
   0 (both already hold seats) and read tbl's live length; a donor whose priority
   rises but stays in the same rank position is acceptable, one that adds a new
   register claimant is not. Use tmp/grind/func_80041188/s9/run.sh - it prints
   the priority table AND the seat verdict in one ~1s run.
3. **The MATRIX * respelling (carried over from s8, still unspent).** a4
   addresses two adjacent 0x20-byte objects, PsyQ's MATRIX is 0x20 bytes, and
   func_8004A348(buf, m) / func_800523E0(m0, m1, a3, dst) read as matrix
   construction and consumption. Spelling pa4/out2/out3 as `MATRIX *m = (MATRIX *)a4;`
   with `&m[0]` / `&m[1]` would make the between-loops re-take of `&m[1]` read as
   ordinary scoping rather than as a dead-store re-init, which materially changes
   the layer-1 exposure of any form built on the I2 chassis.
   *Next probe:* re-spell candidate.c and the M1 variant with a MATRIX-typed (or
   equivalent 0x20-byte struct) local and confirm the .lreg table is unchanged;
   if it is, adopt that spelling everywhere before stacking any further construct.

## [s9] s8 frontier #1 - the separated-staging mechanism works inside block 0 too, and three such staging insns placed after tbl's definition raise tbl's reg_live_length from 47 to 50 byte-free, putting tbl at 12/50 = 2400 tied with i at 24/100 = 2400 and won by i on allocno 78 < 79.
- mechanism: cse1 substitutes a SET_SRC into the immediately following insn but not across an intervening statement, so a separated two-step definition survives cse1/cse2 into flow (where reg_n_refs and reg_live_length are fixed at flow.c:2081/1685) and is merged back by combine afterwards, costing no bytes; each such surviving insn after tbl's definition should add +1 to tbl's live length.
- probe: Built an extraction-TU harness with a mechanical seat checker (tmp/grind/func_80041188/s9/run.sh parses ';; Register dispositions:' out of .greg and prints SEATS ALL-TARGET or the wrong seats) and swept nineteen block-0 variants on the I2 chassis: the sym-K advance in five slots, the stptr split-init in three placements, the saved split-init alone and combined with stptr's, a base two-step, a fresh-intermediate copy, and three placements of `i = 1`. Read 'Register 79 used N times across M insns' from .lreg for each.
- result: The '+1 live per staging insn' prediction is FALSE. Separated staging does survive in block 0, but only in one spelling, and tbl's reg_live_length never reaches 48 from any of them - every variant reads 45 or 47. The single variant that did produce the winning arithmetic (tbl 6/48 = 2500, i 8/96 = 2500, tie won by allocno 78 < 79, stptr still top at 3414) got there by lifting `saved` from 2 refs / pri 425 to 5 refs / pri 2083, which makes saved a TENTH claimant on nine callee-saved seats: global_alloc seats saved and spills a3, where target seats a3 in $fp and spills saved. Two further spellings were killed with named mechanisms - a fresh intermediate is copy-propagated by cse1 and never counted, and a constant two-step is constant-folded.
- verdict: KILLED

## [s9] s8 frontier #2 - tbl reaches the priority window from the 5-reference side at live 41, which requires its definition to be the last insn before loop1; the six block-0 insns that follow the definition are what pin the count, and moving those above it is a lever distinct from moving the definition.
- mechanism: reg_live_length counts the insns in which a pseudo is live (flow.c:1685, one increment per insn per live pseudo, walking RTL order); H-s8-1 proved the source position of tbl's assignment cannot move the count, so the pin must be the trailing block-0 insns.
- probe: Swept the position of the sym-K ADVANCE insn (which is what starts tbl's counted live range once the split-init exists) across every slot of block 0, and separately swept the position of `i = 1` across three slots that keep it before tbl's definition.
- result: Stronger than the frontier asked for: tbl's reg_live_length is a ONE-SIDED quantity with a hard CEILING of 47 at 6 refs (= loop1's 41 insns + a fixed 6-insn block-0 tail). The advance's position can LOWER it to 45 (advance placed last) but nothing raises it above 47. `i = 1`'s position inside block 0 is completely inert - three placements give bit-identical .lreg tables (i 8/95 = 2526, tbl 6/47 = 2553). Nine distinct block-0 mutations across s9, plus s8's tbl-assignment sweep and s6's 16-position tbl++ sweep, and the only mutation in the whole ledger that has ever produced tbl live 48 is moving `i = 1` past tbl's definition.
- verdict: KILLED

## [s9] Target's preamble emission order and tbl's winning live length can be obtained simultaneously, because sched1 runs after flow and its ordering could be steered independently of the RTL order that flow measures.
- mechanism: flow_analysis (toplev.c:2983) fixes reg_live_length from the RTL insn order; sched1 (toplev.c:3033) decides emission order from INSN_PRIORITY, then a dependence class relative to last_scheduled_insn, then INSN_LUID. Separating the two definition insns on priority or class would decouple live length from emission order.
- probe: Read sched.c rank_for_schedule end-to-end and the function's own .sched dump for the priorities, ref_counts and ready-list trace of the two definition insns.
- result: KILLED for every lever reachable from C. Both defs are leaves - .sched reports 'priority = 1, ref_count = 0' for each, because neither has an in-block consumer - so the priority test ties; neither can take a dependence class below 3 (i's def is a `li` with no inputs and so cannot be data-dependent, and nothing else writes either pseudo in block 0), so the class test ties; and the final `return INSN_LUID (tmp) - INSN_LUID (tmp2)` fallback, with GCC 2.7.2 scheduling each block BACKWARD, reproduces RTL order exactly. Raising tbl's def priority would require an in-block consumer surviving combine into sched1, which by definition materialises a byte. s7's measured anti-correlation law now has its named mechanism and is structural, not incidental.
- verdict: KILLED

## [s9] i's live length can be shortened enough for the i-FIRST definition order (the one that yields target's preamble emission) to seat tbl, by exploiting the dead hole in i's live range between loop1's exit test and its re-definition `i = 0x12`.
- mechanism: flow.c:1685 counts only insns in which the pseudo is live; i is dead across the leading part of the between-loops block, so moving its re-definition later grows that hole and shortens reg_live_length[i] without touching any other pseudo.
- probe: Four-position sweep of `i = 0x12;` inside the 6-statement between-loops block on the I2+symK chassis, plus a left-edge control that ADDS a demonstrably-surviving insn to the hole (a separated self-update of stptr2).
- result: CONFIRMED as a lever, KILLED as a standalone solution, and new to the ledger. The sweep works: i live 97 (committed 3rd position) -> 96 (4th) -> 95 (5th) -> 95 (6th/last), i.e. pri 2474 -> 2500 -> 2526. But 95 is the floor and tbl's ceiling of 6/47 = 2553 still outranks it by 27 points, so tbl steals $s4. The control proves the hole is one-sided: the stptr2 self-update is a real added insn (stptr2 6 -> 8 refs, a1/a2 live 99 -> 100) yet i stays at 8/95, because it lands where i is already dead.
- verdict: CONFIRMED

## [s9] i can be given a ninth flow-counted reference, putting it at 27/95 = 2842 and closing the i-FIRST order outright without touching tbl, out2, stptr, stptr2, saved, pa4 or a3.
- mechanism: reg_n_refs is fixed by flow before combine runs, so a reference in an insn combine later deletes is free; floor_log2(9)*9 = 27 against floor_log2(8)*8 = 24 is a 12.5% numerator jump, and an insn placed in i's dead hole costs no live length at all.
- probe: `i = 0x10; a2 += 0x6C; i += 2;` in the between-loops block - a separated SELF-update, the shape that survives for register-valued pseudos - measured against the plain `i = 0x12;` in .lreg.
- result: KILLED for the constant spelling: cse1 constant-folds the pair before flow sees it and i stays at 8 refs / live 97, bit-identical to the plain form. This pins down the third case of the refined cse1 survival rule - self-updates survive only when the first step is REGISTER-valued. The family is not closed: a register-valued self-update of i has never been tried and is now the single highest-value open probe in the ledger.
- verdict: KILLED

## s10 hypotheses (structural, 2026-08-23)

- **H-s10-1 (KILLED).** *"A surviving block-0 insn whose donor already holds a
  callee-saved seat (`stptr`, `pa4`) gives the i-FIRST order its single missing insn"*
  (s9 frontier 2). Probed `stptr` three-step and four-step splits in two placements and
  a `base` two-step: every one raises the donor's reg_n_refs (stptr 7 -> 9 -> 10, so the
  insns really do survive cse1 into flow) and leaves tbl's reg_live_length at 47 and i's
  at 97. `saved` remains the ONLY donor that moves tbl at all, and it moves it to 48 and
  stops there even at three steps. KILLED - and the reason it could never have been
  enough is H-s10-2.

- **H-s10-2 (CONFIRMED, and it closes the axis).** *"In the i-FIRST definition order,
  `reg_live_length(i) - reg_live_length(tbl)` is a constant."* Measured = 49 across all
  24 K1_L0 grid variants. With tbl 6 refs / i 8 refs that makes the priority test
  `24/(L+49) > 12/L`, i.e. tbl live >= 50, while sixteen distinct block-0 mutations bound
  tbl live at <= 48; and tbl at 5 or 4 refs falls below out2 (pinned at 5 refs / <= 44
  by the O-chassis). So **the i-first + O chassis cannot seat an 8-reference `i` in
  $s4** by any block-0 mutation. The entire s9 frontier-2 lever family is dead.

- **H-s10-3 (CONFIRMED).** *"A ninth flow-counted reference to `i` closes the i-FIRST
  order outright"* (s9 frontier 1). Confirmed by direct measurement: i 9/99 = 2727 vs
  tbl 6/47 = 2553, SEATS ALL-TARGET, and `sandbox --disable all` = 3 at 133 insns - one
  insn more than target, that insn being the reference itself. The axis pays and the
  price is now known exactly.

- **H-s10-4 (KILLED).** *"`stptr2 = saved + i * 0x68` after `i = 0x12` supplies the
  ninth reference for free"* - the arithmetic is exact but cse1 constant-folds it and
  the reference is never counted. Any ninth reference must be placed where cse1 does not
  know i's value: BEFORE the `i = 0x12;` reset, i.e. reading loop1's exit value.

### s10 frontier (in priority order)

1. **A BYTE-FREE ninth flow-counted reference to `i`, read before the `i = 0x12` reset,
   finishes the function.** Mechanism: flow.c:2081 fixes reg_n_refs before combine runs,
   so a reference that survives cse1 into flow but whose insn combine later deletes costs
   zero bytes - exactly the way the s8 separated out2 restore already buys out2 its
   extra reference at no byte cost (E-s8, E-s10-4). Next probe: enumerate spellings in
   which loop1's exit value of `i` is CONSUMED by an expression combine can fold away -
   e.g. feeding `i` into an address computation that combine re-associates back to a
   constant offset, or into a comparison combine merges with loop1's own `slti`. Read
   `.combine` (`pwsh tools/grinder/dump.ps1 func_80041188`) for each: the test is
   "reg 78 used 9 times" in `.lreg` AND 132 build insns in the sandbox. The measured
   base form is `tmp/grind/func_80041188/s10/g/K1_L0_ABOPI.c` at sandbox 3 / 133 insns.

2. **The sym-K tbl lift (4 -> 6 refs) is an invention that a corrected chassis may not
   need.** Mechanism: E-s10-6 reads the honest reference vector straight off target -
   i 8, tbl 4, out2 3, out3 3, and a single `addiu $s3,$v0,0xFC` for stptr (our chassis
   splits it). Every lever the last four sessions have spent exists only to compensate
   for out2 sitting at 5 refs / 42 in the O-chassis; with tbl at its honest 4 refs the
   requirement is out2 < 1702, i.e. out2 live > 58, which nothing in the current shape
   can reach. Next probe: attack out2's live length rather than tbl's priority - find a
   chassis in which the loop1 pointer is still live at the END of the between-loops
   block (as it is in candidate.c, 4 refs / 47) WHILE the between-block still emits
   `addiu $s3,$s7,0x20` rather than `move $s3,$s6`. That is the same one-insn question
   as frontier 1, asked from the out2 side instead of the i side.

3. **Spelling pa4/out2/out3 through a real PsyQ `MATRIX *` local is byte-neutral and
   materially reduces layer-1 exposure** (carried from s8/s9, still unspent - it is
   cosmetic-for-the-Judge, not a floor lever, so it should be applied only once a
   distance-0 form exists). Mechanism and probe unchanged: a4 addresses two adjacent
   0x20-byte objects, PsyQ's MATRIX is 0x20 bytes, and `&m[0]` / `&m[1]` reads as
   ordinary scoping rather than as a re-init. Confirm the .lreg table and seat verdict
   are unchanged before adopting.

## [s10] A surviving block-0 insn whose donor already holds a callee-saved seat (stptr, pa4, base) supplies the i-FIRST order's single missing insn and lifts tbl to live 48 (s9 frontier 2).
- mechanism: flow.c:1685 increments reg_live_length once per insn for every pseudo live at that insn, so an insn added after tbl's definition in block 0 should lengthen tbl's range; E-s9-8 required the donor to already hold a seat so as not to create a tenth claimant on nine callee-saved seats.
- probe: stptr three-step and four-step separated splits in two placements (A1, A2), a base two-step (A3), an out2 two-step (D4) and a saved three-step (D2), each measured through the s8/s9 extraction harness for the full .lreg allocno priority table plus the .greg seat verdict.
- result: stptr's reg_n_refs rises 7 -> 9 (A1) and 7 -> 10 (A2), proving the extra insns survive cse1 into flow, yet tbl stays at live 47 and i at live 97 in both placements; the base and out2 splits likewise leave tbl at 47 or lower (D4: 46). saved remains the only donor that moves tbl at all, and it moves it to 48 and stops there even with three surviving steps (D2: saved 7 refs / pri 2916, a3 spilled).
- verdict: KILLED

## [s10] In the i-FIRST definition order, reg_live_length(i) - reg_live_length(tbl) is a constant, so the i-vs-tbl priority contest is decided by tbl's live length alone.
- mechanism: With i defined before tbl, every block-0 insn after tbl's definition lies inside BOTH live ranges; i additionally covers its own definition insn, the between-block tail and the whole of loop2, and none of those regions overlaps tbl's range, so flow.c:1685 counts a fixed excess for i.
- probe: 112-variant grid (tmp/grind/func_80041188/s10/gen.py + batch.sh) over every interleaving of `i = 0x12` and the s8 separated out2 restore with a1/a2/stptr2 in the between-loops block, crossed with sym-K on/off and `i = 1` first/last in block 0, reading the allocno priority table and seat verdict for each.
- result: The difference is exactly 49 in all 24 i-first variants (tbl 47 / i 96 with `i = 0x12` last, rising to 97/98/99 as it moves earlier). With tbl at 6 refs and i at 8 the priority test 24/(L+49) > 12/L requires tbl live >= 50, while sixteen distinct block-0 mutations across s9 and s10 bound tbl live at <= 48. tbl at 5 refs (2127) or 4 refs (1702) instead falls below out2, which the separated-restore chassis pins at 5 refs / live 42-44 = 2272-2380 and which cannot go below 1702 without live > 58. The i-first + separated-restore chassis is therefore arithmetically CLOSED for an 8-reference i.
- verdict: CONFIRMED

## [s10] A ninth flow-counted reference to `i` closes the i-FIRST order outright and needs no change to tbl, out2, stptr, stptr2, saved, pa4 or a3 (s9 frontier 1).
- mechanism: floor_log2(9)*9 = 27, so i moves to 27/99 = 2727 against tbl's 6/47 = 2553 ceiling, in the definition order that already produces target's preamble emission (sched.c rank_for_schedule INSN_LUID fallback with backward block scheduling, E-s9-3).
- probe: K1_L0_ABOPI (sym-K tbl + `i = 1` first + the s8 separated between-loops out2 restore + `i = 0x12` last) with `stptr2 = saved + 0x750 + (i - 0x12);` placed before the reset so cse1 cannot fold it; seat checker plus `sandbox func_80041188 --disable all`.
- result: i 9 refs / live 99 = 2727 > tbl 6/47 = 2553; SEATS ALL-TARGET - the first form in the ledger where the i-FIRST preamble order and the separated-restore slot-72 `addiu $s3,$s7,0x20` co-exist with every callee-saved seat equal to target. sandbox --disable all = 3 at 133 build insns vs 132 target: the entire residual is the one insn computing the (i - 0x12) zero. The spelling itself is a semantic no-op (cheat-checklist T1/T2/T3 all fail) and is banked as proof the axis pays, not as a candidate.
- verdict: CONFIRMED

## [s10] `stptr2 = saved + i * 0x68;` written after `i = 0x12;` supplies the ninth reference for free, since 18 * 0x68 == 0x750 exactly.
- mechanism: A register-valued operand should survive cse1 into flow per the E-s9-9 survival rule and be re-folded by combine before any byte is emitted.
- probe: Variant E2 through the extraction harness; compared the full .lreg table against the unmodified K1_L0_ABOPI form.
- result: cse1 already knows i == 0x12 at that point and constant-folds the multiply; the .lreg table comes back bit-identical (i 8/97 = 2474, tbl 6/47 = 2553, i and tbl still swapped on $s4/$s5). The E-s9-9 survival rule does not protect an operand whose VALUE cse knows: any ninth reference must read loop1's exit value, before the `i = 0x12;` reset.
- verdict: KILLED

## [s11] With `out3 = pa4 + 0x20` (target's slot-72 insn), out2's definition can be moved to the head of block 0 to lengthen its live range into the priority window (1458, 1702) that target's seating requires.
- mechanism: global.c allocno_compare pri = floor_log2(n_refs)*n_refs/reg_live_length*10000; flow.c:1685 counts one live insn per RTL insn a pseudo spans, so an earlier definition lengthens the range without adding a reference. The window's lower bound is pa4, which gains out3's reference on this chassis (7 refs / 96 = 1458); the upper bound is tbl (4 / 47 = 1702).
- probe: out2's definition made the FIRST statement of block 0 with `out3 = (s32 *)((u8 *)pa4 + 0x20)`; read the full .lreg priority table via tmp/grind/func_80041188/s10/batch.sh (under WSL) and ran sandbox --disable all.
- result: out2 measures 3 refs / live 43 - the maximum reachable, since block 0 holds only two insns above out2's definition and out2 dies at loop1's exit. sandbox 18 at 132 insns, seats out2=$fp / pa4=$s6 / a3=$s7. The window needs live in (47.0, 54.9) at 4 refs; 43 gives 1860 (above tbl) and 3 refs gives 697 (below pa4). ARITHMETICALLY EMPTY.
- verdict: KILLED

## [s11] The F1 combine-foldable chain-extender (dead-store-fake-exception.md:32, owner ruling 2026-07-01) - the last unspent sanctioned byte-free reg_n_refs family - can give out2 a 4th flow-counted reference inside block 0, where (unlike the between-loops block of E-s8) combine CAN see out2's definition and fold the detour away.
- mechanism: flow.c:2081 fixes reg_n_refs before combine runs (toplev.c:2983 vs 3004), so a detour insn that survives cse1 into flow and is folded by combine costs zero bytes; combine's LOG_LINKS are intra-block, and out2's definition and the detour are both in block 0.
- probe: `saved = (s32)out2 + (base + 0x94 - 0x20 - (s32)pa4);` replacing `saved = base + 0x94;` (algebraically exact because out2 == pa4 + 0x20), measured with and without the s4/s5 sym-K tbl lift, and with the detour placed both immediately after out2's definition and as the last statement of block 0. .lreg table + sandbox for each.
- result: out2 does reach 4 refs (live 46 = 1739), so the counting premise holds in block 0. But the detour necessarily references pa4 to cancel - out2 is the only pa4-derived value in scope - taking pa4 to 8 refs / 99 = 2424, ABOVE out2, which permutes exactly the seats it was meant to fix (sym-K on top gives tbl 6/49 = 2448 and does not change that). And it MATERIALIZES: sandbox 27 at 134 build insns vs target's 132, so F1's own zero-bytes prerequisite fails outright. The only pa4-free compensation is the forbidden `out2 - out2` cancellation pair.
- verdict: KILLED

## [s11] candidate.c is "ordinary C" as its header claims.
- mechanism: n/a - audit of the shipping candidate against the F1 definition, prompted by E-s10-6's honest-reference-vector finding that target emits a single `addiu $s3,$v0,0xFC` where candidate.c writes `stptr = base; stptr += 0xFC;`.
- probe: un-split it to `stptr = base + 0xFC;` and measure .lreg + sandbox.
- result: FALSE. stptr falls from 7 refs / 41 = 3414 to 5 / 41 = 2439, below stptr2's 2500; seats permute and the floor goes 1 -> 15, while the build stays 132 insns either way. The split is byte-neutral and its only surviving effect is the reg_n_refs count - i.e. candidate.c already carries an UN-ANNOTATED F1 combine-foldable chain-extender, which a layer-1 reviewer would FAIL. Any future candidate on this chassis owes a /* FAKE: ... */ annotation on that split (or a replacement for it).
- verdict: CONFIRMED (as a defect in the candidate, not as a lever)

## [s11] With `out3 = (s32 *)((u8 *)pa4 + 0x20)` (the only chassis that emits target's slot-72 `addiu $s3,$s7,0x20`), out2's definition can be moved to the head of block 0 to lengthen its live range into the priority window (1458, 1702) that target's callee-saved seating requires.
- mechanism: global.c allocno_compare pri = floor_log2(n_refs)*n_refs/reg_live_length*10000; flow.c:1685 counts one live insn per RTL insn a pseudo spans, so an earlier definition lengthens the range without adding a reference. The window's lower bound is pa4, which gains out3's reference on this chassis (7 refs / 96 = 1458); the upper bound is tbl (4 refs / 47 = 1702).
- probe: out2's definition made the FIRST statement of block 0 with out3 taken from pa4; full .lreg priority table read via tmp/grind/func_80041188/s10/batch.sh (run under WSL) plus `sandbox func_80041188 --disable all`.
- result: out2 measures 3 refs / live 43 - the maximum reachable, since only two insns sit above its definition in block 0 and it dies at loop1's exit. sandbox 18 at 132 insns; seats out2=$fp, pa4=$s6, a3=$s7. The window needs live 47..55 at 4 refs; live 43 gives 1860 (above tbl) at 4 refs and 697 (below pa4) at 3 refs.
- verdict: KILLED

## [s11] The F1 combine-foldable chain-extender (.claude/rules/dead-store-fake-exception.md:32, owner ruling 2026-07-01) - the last unspent sanctioned byte-free reg_n_refs family - can give out2 a 4th flow-counted reference inside block 0, where (unlike the between-loops block killed in s8) combine can see out2's definition and fold the detour away for zero bytes.
- mechanism: flow.c:2081 fixes reg_n_refs before combine runs (toplev.c:2983 vs 3004), so a detour insn that survives cse1 into flow and is folded by combine costs nothing; combine's LOG_LINKS are intra-block and out2's definition and the detour are both in block 0.
- probe: `saved = (s32)out2 + (base + 0x94 - 0x20 - (s32)pa4);` replacing `saved = base + 0x94;` (algebraically exact because out2 == pa4 + 0x20), measured with and without the s4/s5 sym-K tbl lift and with the detour both immediately after out2's definition and last in block 0; .lreg table + sandbox for each.
- result: The counting premise holds - out2 does reach 4 refs / live 46 = 1739. But the detour must subtract a pa4-derived term to cancel (out2 is the only pa4-derived value in scope), taking pa4 to 8 refs / 99 = 2424, ABOVE out2, permuting exactly the seats it was meant to fix; sym-K on top gives tbl 6/49 = 2448 and does not help. And it MATERIALIZES: sandbox 27 at 134 build insns vs target's 132, failing F1's own explicit zero-bytes prerequisite.
- verdict: KILLED

## [s11] candidate.c is 'ORDINARY C ... no FAKE construct' as its own header claims.
- mechanism: Audit against the F1 definition, prompted by E-s10-6's honest-reference-vector reading that target emits a single `addiu $s3,$v0,0xFC` (asm/funcs/func_80041188.s:26) where candidate.c writes `stptr = base; stptr += 0xFC;`.
- probe: Un-split it to `stptr = base + 0xFC;`; .lreg table + sandbox.
- result: FALSE. stptr falls from 7 refs / 41 = 3414 to 5 / 41 = 2439, below stptr2's 2500; seats permute and the floor goes 1 -> 15 while the build stays 132 insns either way. The split is byte-neutral and its only surviving effect is the reg_n_refs count - i.e. candidate.c already carries an UN-ANNOTATED F1 combine-foldable chain-extender that a layer-1 reviewer would FAIL.
- verdict: CONFIRMED

## [s12] Frontier #1 — a single PsyQ `MATRIX *` local addressing both matrices (`&m[0]` / `&m[1]`) changes out2's live-range endpoints, because a scaled-index address expression is a different RTL address form than the `pa4 + 0x20` plus.
- mechanism (as carried from s8/s9/s11): a4 addresses two adjacent 0x20-byte objects and PsyQ's `MATRIX` is exactly 0x20 bytes (`include/gte.h:29`), so `MATRIX *m = (MATRIX *)a4;` with `&m[0]`/`&m[1]` would make loop2's pointer an ordinary re-scoped address expression rather than `out3 = out2` or `out3 = pa4 + 0x20`. The claim was that this is the only change that can move out2's live-range ENDPOINTS rather than just its reference count.
- probe: `tmp/grind/func_80041188/s12/A_matrix.c` — `MATRIX *m = (MATRIX *)a4;`, every `pa4` spelled `(s32 *)&m[0]`, every `out2`/`out3` spelled `(s32 *)&m[1]`, the `out2`/`out3` locals and their definitions deleted. Measured with `tmp/grind/func_80041188/s10/batch.sh` (.lreg priority table + .greg seat verdict) and `sandbox func_80041188 --disable all`.
- result: **sandbox 43, 132 target / 125 build insns.** The chassis loses SEVEN instructions. `&m[1]` is not a new RTL address form at all — the C front end folds the index to the same `(plus (reg) (const_int 32))` the `pa4 + 0x20` spelling produces — so cse1 collapses all four occurrences into ONE value with ONE pseudo, which is exactly the single-local chassis s7 already disproved (GCC 2.7.2 has no live-range splitting, so one pseudo cannot occupy `$s6` across loop1 and `$s3` across loop2). The .lreg table shows the collapse directly: only 8 pseudos survive instead of 11, `out2`/`out3` are gone as separate allocnos, and `pa4` rises to 8 refs / live 94 (from 6 / 95) because `&m[0]` is now written inline at all four call sites. Live-range ENDPOINTS are unchanged by the typedef; the frontier's premise was wrong.
- verdict: KILLED — banked as `rejected/matrix-single-local-collapses-out2-out3-125insn.c`. Frontier item #1 (carried unspent since s8) is retired.

## [s12] The out2 priority window can be entered from BELOW the required live length — i.e. by MINIMISING out2's live range instead of maximising it — by defining out2 inside loop1 rather than in block 0.
- mechanism: global.c's allocno priority is `floor_log2(reg_n_refs) * reg_n_refs / reg_live_length * 10000`, so priority is INVERSE in live length. Every prior session (s7 order sweep, s11 block-0 ceiling) pushed out2's live length UP to reach target's window; the window is a band, not a floor, so the same band is also reachable from a SHORT live range. At 3 references the band (1263, 1702) — strictly between `pa4` (6/95 = 1263) and `tbl` (4/47 = 1702) — corresponds to live length 17.6 .. 23.7, which only a definition inside loop1's body can produce.
- probe: four spellings on the `out3 = (s32 *)((u8 *)pa4 + 0x20)` chassis (the ONLY chassis that emits target's `addiu $s3,$s7,0x20`), with out2's definition moved out of block 0 and into loop1's body at four different positions: `B0_loop1top` (first statement of the body), `B1_beforecall1`, `B2_aftercall1`, `B3_midlate`. Read .lreg refs/live + .greg dispositions for all four; scored B0 in the sandbox.
- result: **all four are byte-identical in .lreg and .greg — in-loop definition POSITION is completely inert** (cse1 normalises the definition to the same place regardless of where it is written). out2 comes out at **4 references / live length 21 = priority 3809**, not the 3 refs the source shape suggests, and 3809 is more than twice tbl's 1702 — it does not land inside the band, it flies far above the TOP of it. Seats permute wholesale (out2 takes `$s3`, tbl is pushed to `$s6`, stptr to `$s4`). `B0_loop1top` scores **sandbox 23 at 132 insns**.
- verdict: KILLED — banked as `rejected/out2-def-inside-loop1-live21-pri3809-overshoots-window.c`. Together with s11's ceiling this closes the window from BOTH sides; see evidence E-s12-3 for the complete enumeration.

## [s12] Frontier #1 (carried unspent since s8): a single PsyQ MATRIX * local addressing both matrices as &m[0] / &m[1] changes out2's live-range ENDPOINTS, because a scaled-index address expression is a different RTL address form than the pa4 + 0x20 plus, and that is the one input to the allocno-priority arithmetic every measured chassis holds fixed.
- mechanism: a4 addresses two adjacent 0x20-byte objects and PsyQ's MATRIX is exactly 0x20 bytes (include/gte.h:29); func_8004A348(buf, m) builds a rotation matrix and func_800523E0(m0, m1, a3, dst) consumes the pair, so a MATRIX-typed local would make loop2's pointer an ordinary re-scoped address expression rather than out3 = out2 or out3 = pa4 + 0x20.
- probe: tmp/grind/func_80041188/s12/A_matrix.c -- MATRIX *m = (MATRIX *)a4; every pa4 spelled (s32 *)&m[0], every out2/out3 spelled (s32 *)&m[1], both pointer locals and their definitions deleted. Read the .lreg priority table and .greg seat verdict via tmp/grind/func_80041188/s10/batch.sh under WSL, then scored with sandbox --disable all.
- result: sandbox 43, 132 target / 125 build insns -- the chassis LOSES seven instructions. &m[1] is not a new RTL address form at all: the C front end folds the array index to a constant byte offset, producing the identical (plus (reg) (const_int 32)), so cse1 collapses all four occurrences into ONE value. The .lreg table shows the collapse directly (8 pseudos instead of 11; out2 and out3 cease to exist as separate allocnos; pa4 rises to 8 refs / live 94 from 6 / 95 because &m[0] is now inline at four call sites). This is the single-local chassis s7 already disproved -- GCC 2.7.2 has no live-range splitting, so one pseudo cannot hold $s6 across loop1 and $s3 across loop2.
- verdict: KILLED

## [s12] The out2 allocno-priority window can be entered from BELOW the required live length -- by MINIMISING out2's live range (defining it inside loop1) rather than maximising it, which is the only direction s7/s11 ever pushed.
- mechanism: global.c's priority is floor_log2(reg_n_refs) * reg_n_refs / reg_live_length * 10000, i.e. INVERSE in live length, and target's requirement is a BAND (1263, 1702) between pa4 and tbl, not a floor. At 3 references that band corresponds to live length 17.6..23.7, which only a definition inside loop1's body can produce.
- probe: Four spellings on the out3 = (s32 *)((u8 *)pa4 + 0x20) chassis -- the ONLY chassis that emits target's addiu $s3,$s7,0x20 -- with out2's definition moved out of block 0 into loop1's body at four positions (B0_loop1top, B1_beforecall1, B2_aftercall1, B3_midlate). Read refs/live from .lreg and dispositions from .greg for all four; scored B0 in the sandbox.
- result: All four are BYTE-IDENTICAL in .lreg and .greg -- in-loop definition position is completely inert (cse1 normalises the definition to one place). out2 comes out at 4 references / live length 21 = priority 3809, not the 3 refs the source shape suggests, and 3809 is more than TWICE tbl's 1702: it does not land inside the band, it flies far above its top. Seats permute wholesale (out2 -> $s3, tbl -> $s6, stptr -> $s4). B0_loop1top scores sandbox 23 at 132 insns.
- verdict: KILLED

## [s12] COMPLETING s11's proof: target's out2 allocno priority is reachable ONLY with a reference to out2 in the between-loops block, and that reference is necessarily the residual instruction itself.
- mechanism: pa4 (6 refs / live 95 = 1263) and a3 (4 / 99 = 808) are pinned by the call signatures; tbl is pinned at 4 / 47 = 1702 with live bounded at 48 (s10, sixteen block-0 mutations). With pri = floor_log2(n)*n/live*10000 the required band (1263, 1702) admits exactly (n=3, live 17.6..23.7), (n=4, live 47.0..54.9), (n=5, live 58.7..79.2), (n=6, live 70.5..95.0).
- probe: Enumerated out2's ACHIEVABLE (refs, live) pairs across every measured chassis: block-0 definition with no post-loop1 reference (s7 order sweep + s11 ceiling), definition inside loop1 (this session, four positions), the single-local chassis (s7 + this session's MATRIX probe), and block-0 definition WITH a between-loops reference (candidate.c).
- result: block-0 definition, no post-loop1 reference: live pinned to 42-43 -> n=3 gives 714/697 (below pa4 AND a3; sandbox 15), n=4 gives 1904/1860 (above tbl, steals $s5; sandbox 10); the band's n=4 requirement of live >= 47 is unreachable because nothing after loop1 references out2 on that chassis. Definition inside loop1: 4 refs / live 21 = 3809 (sandbox 23). n=5 / n=6 need live 59..95, i.e. out2 live through loop2 = the single-local chassis (125 insns, sandbox 43). Block-0 definition WITH a between-loops reference: live 47, n=4 = 1702.1, inside the band, tie with tbl broken the right way by allocno number 79 < 86 -- this is candidate.c at floor 1 with ALL-TARGET seats, and that reference IS the residual insn (move $s3,$s6 vs target addiu $s3,$s7,0x20). Combined with s8 (combine's LOG_LINKS are intra-block, and the six insns target emits in the between-loops block consume neither $s6 nor any out2-derived value, so the only possible consumer of an out2 reference there is out3's own definition) the residual is a fixed point of the priority arithmetic, not an unfound spelling.
- verdict: CONFIRMED

## [s13] The floor-9 chassis (s4's distance-0 form minus the banned do-while(0) wrap) has a NON-BANNED route to distance 0 inside the permuter's transform set — i.e. the wrap was one of several possible closers and s4 merely happened to find that one first.
- mechanism: the floor-9 chassis's residual is a pure `$s5`/`$s6` seat swap between `tbl` (4 refs / live 47 = 1702.1) and `out2` (5 refs / live 42 = 2381.0) under global.c's `floor_log2(reg_n_refs) * reg_n_refs / reg_live_length * 10000`, and register-shaped residuals are exactly what decomp-permuter's weighted metric hill-climbs (s4's own rule). If any other statement-level mutation re-weights that pair, the permuter's ~10k-iteration-per-8-minute throughput should find it.
- probe: two campaigns on the same base (`tmp/grind/func_80041188/s13/perm_f9` and `.../perm_f9b`, permuter base score 63, honest sandbox 9, 132/132). Campaign 1 with the stock transform set; campaign 2 identical except `settings.toml` carries `[weight_overrides] perm_ins_block = 0`, which zeroes the ONLY randomizer that can emit `do { ... } while (0)` (randomizer.py:1092, default weight 10 at default_weights.toml:19).
- result: **KILLED.** Campaign 1 reached distance 0 in **2,588 iterations / 125 s**, and its `output-0-1` differs from the base by exactly the BANNED construct, re-derived verbatim (`do { loop1: … func_8004A348(buf, out2); } while (0);`, label inside the wrap, back edge entering from outside). Campaign 2, with block insertion forbidden, ran **22,418 iterations / 1,121 s and produced ZERO finds** — not one candidate scored below the base 63. An 8.7x larger search with the wrap removed does not even improve the base, let alone close it. Within the permuter's transform set the banned wrap is not *a* closer for this chassis, it is the *only* one.
- verdict: KILLED — artifacts `tmp/grind/func_80041188/s13/perm_f9/`, `.../perm_f9b/`, find banked as `tmp/grind/func_80041188/s13/f9_zero_banned_wrap.c` (NOT re-proposed).

## [s13] The two-locals `out3 = (s32 *)((u8 *)pa4 + 0x20)` chassis is an INDEPENDENT permuter basin — it emits target's `addiu $s3,$s7,0x20` natively and its residual is a pure {out2, pa4, a3} seat permutation, the residual shape s4 proved the permuter closes, yet it has never been permuted.
- mechanism: s4's won campaign started from the SINGLE-local pa4-read chassis at permuter base 88 and climbed 88 -> 63 -> 0. The two-locals analogue (`rejected/two-locals-out3-from-a4-seat-permutation.c`, honest sandbox 15) measures the identical permuter base 88 and is structurally 1:1 with target across all 132 insns, so if the basins are distinct the same climb should be available from a chassis that does not require s4's `out2` staging store.
- probe: `tmp/grind/func_80041188/s13/perm_2l`, `perm_ins_block = 0` so any find is wrap-free by construction; 8 jobs; harvested with `--stop`. Every find hand-re-measured in the honest sandbox rather than trusted.
- result: **KILLED.** 21,853 iterations / 1,100 s produced exactly ONE find (score 63), and it arrived in the first 29 seconds with nothing better in the remaining 18 minutes. The find is `out2 = (s32 *)(((u8 *)pa4) + 0x20);` re-executed as a between-loops statement immediately before `out3 = out2;` — a SAME-VALUE re-store of `out2` (dead-store family, FAKE-requiring) whose only effect is to buy `out2` the between-loops reference the priority arithmetic demands, i.e. the permuter independently re-derived E-s12-3's conclusion from the other direction. Honest sandbox: **9, 132/132 — identical to the floor-9 chassis of campaigns 1-2.** The basin is not independent; it drains into the floor-9 basin, which campaign 2 then proved has no non-`ins_block` exit.
- verdict: KILLED — banked as `rejected/twolocals-permuter-samevalue-restore-funnels-to-floor9-basin.c`.

## [s13] The loop1-as-a-real-do-while chassis (honest sandbox 5 — the lowest-distance non-candidate form in the bank, and the only one whose allocno priority ORDER already matches target) is the best permuter seed available, because low honest distance means few instructions to fix.
- mechanism: s8/s9's reading was that this chassis is "one loop.c artefact away" — its five residual instructions are all loop.c induction-variable rewriting (`addiu $s3,$v0,308` vs target `252`; `move $a3,$s3` + `sh $t0,-50($s3)` vs target `addiu $a3,$s3,56` + `sh $v0,6($s3)`) plus the shared `move $s3,$s6` residual, with the seating already correct.
- probe: `tmp/grind/func_80041188/s13/perm_lr`, `perm_ins_block = 0`, 8 jobs.
- result: **KILLED, and the premise was wrong.** The permuter's weighted base score for this chassis is **420** — nearly 5x the 88 of the seat-permutation chassis and 6.7x the 63 of the floor-9 chassis — because reordering diffs cost 60 apiece in its metric while a wrong register costs 5. 4,737 iterations produced 6 finds with a best of **355**: the campaign never approached target. Strength-reduction-shaped residuals are invisible to this search. Honest sandbox distance does NOT predict permuter tractability; residual SHAPE does, which is s4's rule confirmed from the opposite side.
- verdict: KILLED

## [s13] The floor-9 chassis (s4's distance-0 form minus the banned do-while(0) wrap; honest sandbox 9, 132/132) has a NON-BANNED route to distance 0 inside decomp-permuter's transform set -- i.e. the wrap was one of several possible closers and s4 merely found it first.
- mechanism: The chassis's residual is a pure $s5/$s6 seat swap between tbl (4 refs / live 47 = 1702.1) and out2 (5 refs / live 42 = 2381.0) under global.c's floor_log2(reg_n_refs)*reg_n_refs/reg_live_length*10000. Register-shaped residuals are exactly what the permuter's weighted metric (regs x5) hill-climbs, so any other statement-level mutation that re-weights that pair should be reachable at the observed ~10k iterations per 8 minutes.
- probe: Two campaigns on the identical base (permuter base score 63): tmp/grind/func_80041188/s13/perm_f9 with the stock transform set, and tmp/grind/func_80041188/s13/perm_f9b differing ONLY by '[weight_overrides] perm_ins_block = 0' in settings.toml, which zeroes the sole randomizer that can emit do{...}while(0) (tools/decomp-permuter/src/randomizer.py:1092, default weight 10 at tools/decomp-permuter/default_weights.toml:19). Both -j 8, both harvested with --stop.
- result: KILLED. Campaign 1 hit distance 0 in 2,588 iterations / 125 s, and its output-0-1 differs from the base by exactly the BANNED construct re-derived verbatim (do { loop1: ... func_8004A348(buf, out2); } while (0);, the loop1: label INSIDE the wrap, the back edge entering it from outside). Campaign 2, with block insertion forbidden, ran 22,418 iterations / 1,121 s -- 8.7x the search -- and produced ZERO finds: not one candidate scored below the base 63, not even a partial register-shaped improvement. Inside the permuter's transform set the banned wrap is not a closer for this chassis, it is the only closer.
- verdict: KILLED

## [s13] The two-locals out3 = (s32 *)((u8 *)pa4 + 0x20) chassis is an INDEPENDENT permuter basin: it emits target's addiu $s3,$s7,0x20 natively, its residual is a pure {out2, pa4, a3} seat permutation, and it measures the same permuter base 88 as s4's winning chassis -- yet it had never been permuted.
- mechanism: s4's winning campaign climbed 88 -> 63 -> 0 from the SINGLE-local pa4-read chassis. If the two-locals analogue is a distinct basin, the same climb should be available without s4's out2 staging store, giving a wrap-free route to zero on a chassis that is structurally 1:1 with target across all 132 insns.
- probe: tmp/grind/func_80041188/s13/perm_2l seeded from memory/grind/func_80041188/rejected/two-locals-out3-from-a4-seat-permutation.c (honest sandbox 15), with perm_ins_block = 0 so any find is wrap-free by construction; -j 8; harvested with --stop. Every find hand-re-measured in the honest sandbox rather than trusted.
- result: KILLED. 21,853 iterations / 1,100 s produced exactly ONE find (score 63), reached in the first 29 seconds with nothing better in the remaining 18 minutes. The find is out2 = (s32 *)(((u8 *)pa4) + 0x20); re-executed as a between-loops statement immediately before out3 = out2; -- a SAME-VALUE re-store of out2 (dead-store family, FAKE-requiring) whose only effect is buying out2 the between-loops reference the priority arithmetic demands. That is E-s12-3's conclusion, independently re-derived by random search from the other direction. Applied to src/text1a_pre.c and measured: sandbox 9, 132/132 -- byte-identical in distance to the floor-9 chassis. The basin is not independent; it drains into the basin campaign 2 had just exhausted.
- verdict: KILLED

## [s13] The loop1-as-a-real-do-while chassis (honest sandbox 5 -- the lowest-distance non-candidate form in the bank, and the only one whose allocno priority ORDER already matches target) is the best available permuter seed, because few wrong instructions means a short climb.
- mechanism: s8/s9 read that chassis as one loop.c artefact away: its five residual insns are all loop.c induction-variable rewriting (addiu $s3,$v0,308 vs target 252; move $a3,$s3 + sh $t0,-50($s3) vs target addiu $a3,$s3,56 + sh $v0,6($s3)) plus the shared move $s3,$s6, with the callee-saved seating already correct.
- probe: tmp/grind/func_80041188/s13/perm_lr seeded from memory/grind/func_80041188/rejected/loop1-real-do-while-newchassis-score5.c, perm_ins_block = 0, -j 8, harvested with --stop.
- result: KILLED, and the premise was wrong. The permuter's weighted base score for this chassis is 420 -- 4.8x the 88 of the seat-permutation chassis and 6.7x the 63 of the floor-9 chassis -- because reordering diffs cost 60 apiece in its metric while a wrong register costs 5. 4,737 iterations produced 6 finds with a best of 355; the campaign never came within 300 points of closing. Strength-reduction-shaped residuals are invisible to this search. Honest sandbox distance does NOT predict permuter tractability; residual SHAPE does -- s4's rule confirmed from the opposite side.
- verdict: KILLED
