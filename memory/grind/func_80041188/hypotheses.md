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

## [s14] The residual is a SINGLE requirement on out2's allocno priority (the framing every session s7..s13 worked from).
- mechanism: E-s12-3 enumerated out2's achievable (refs, live) pairs against a band (1263, 1702) bounded below by pa4 at 6 refs / 95 and above by tbl at 4 refs / 47, and concluded the only reachable point inside it is the one whose 4th reference IS the residual instruction.
- probe: Reproduced target's honest reference vector exactly (`V1_honest_both` = candidate.c with `stptr = base + 0xFC;` un-split and `out3 = (s32 *)((u8 *)pa4 + 0x20);`) and read the full `.lreg` priority table plus the `.greg` seat verdict through `bash tools/wsl.sh 'bash tmp/grind/func_80041188/s10/batch.sh ...'`, then compared the resulting rank order against target's seats.
- result: FALSE as a single requirement. Target's seats require the strict order stptr > i > tbl > out2 > pa4 > a3, which is TWO independent conditions: (A) stptr > i, and (B) tbl > out2 > pa4 > a3. The honest vector violates BOTH — i 2474 > stptr 2439, and out2 714 < a3 808 < pa4 1473. Additionally the band in (B) is (1473, 1702), not (1263, 1702): on the only chassis that emits target's slot-71 `addiu $s3,$s7,0x20`, pa4 carries out3's reference and measures 7 refs / 95 = 1473. The s12 enumeration's CONCLUSION survives the correction (no achievable out2 pair lands in (1473, 1702) either) but its arithmetic was taken from the wrong chassis.
- verdict: KILLED (as a single-requirement framing); replaced by the two-requirement decomposition in E-s14-1.

## [s14] Requirement (A) — stptr > i — can be satisfied in ordinary C, retiring candidate.c's un-annotated F1 chain-extender `stptr = base; stptr += 0xFC;`.
- mechanism: flow.c:1685 counts one live insn per RTL insn a pseudo spans, and `i` is DEAD in the between-loops block between loop1's last read and the `i = 0x12;` redefinition. Moving the redefinition earlier shortens that dead gap and LENGTHENS reg_live_length(i), which LOWERS its priority `floor_log2(8)*8/live*10000`. At live 99 i measures 2424, below an un-split stptr's 5 refs / 41 = 2439 — so (A) flips without touching stptr at all.
- probe: Four-position sweep of `i = 0x12;` through the between-loops block on the un-split-stptr chassis (`X0_ipos` .. `X3_ipos`), each read for the full `.lreg` priority table and `.greg` seat verdict; `X0_ipos` then spliced into `src/text1a_pre.c` and scored with `sandbox func_80041188 --disable all`, and its residual slots extracted with a normalized objdump comparator (`tmp/grind/func_80041188/s14/cmp.py`).
- result: CONFIRMED. i measures 99/98/97/96 live (2424 / 2448 / 2474 / 2500) at positions 1/2/3/4, and position 1 alone gives ALL-TARGET callee-saved seats with the chain-extender REMOVED — the first form in the ledger with all-target seats and zero FAKE constructs of any kind. It scores 3 at 132/132 insns; the residual is exactly `li $s4,18` at slot 67 instead of slot 69 (pushing the two `addiu $s1/$s2,,0x6C` down one each) plus the long-known slot-71 `move $s3,$s6`. Banked as `memory/grind/func_80041188/alt_fakefree_floor3_s14.c`.
- verdict: CONFIRMED

## [s14] The +2 emission-order cost of the `i = 0x12;`-first fix can be decoupled from its live-length win by extending i's live range at the FRONT (earlier definition) or by shortening stptr's live range instead.
- mechanism: reg_live_length is computed by flow on PRE-sched RTL while emission order is decided by sched1's `rank_for_schedule`, which falls back to INSN_LUID for the three dependency-free between-block insns — both read the same source order, so any decoupling has to come from a different variable than statement position in that block.
- probe: `Y1_ifirst` (`s32 i = 1;` declared first), `Y2_ilate_init` (`s32 i;` declaration split from an `i = 1;` statement placed at the head of block 0), and `Y3_stptr_first` (stptr defined before `saved` and `out2`, to shorten stptr's live range from 41 to 40 and satisfy (A) from stptr's side). Full `.lreg` + `.greg` for each.
- result: KILLED, all three. Y1: i stays 8 refs / live 97 — the parameter copies emitted by `expand_function_start` always precede any user statement, so i's range cannot be extended at the front; only the allocno numbering changes (i becomes r77, pa4 r78). Y2: i 8 / 96 = 2500, worse. Y3: stptr 5 / 44 = 2272, worse — stptr's live range is pinned to loop1's insn count from below and lengthens, never shortens, when its definition moves earlier. The +2 is therefore structural on this chassis.
- verdict: KILLED

## [s14] With requirement (A) satisfied honestly, the fully honest chassis reduces to requirement (B) alone.
- mechanism: n/a — direct measurement of the composed form.
- probe: `Z0_honest_ifirst` = un-split stptr + `out3 = (s32 *)((u8 *)pa4 + 0x20)` + `i = 0x12;` first; `.lreg` table, `.greg` seats, and `sandbox --disable all` after splicing into src.
- result: CONFIRMED. stptr 2439 > i 2424 > tbl 1702 > pa4 1473 > a3 808 > out2 714; stptr, i, tbl, stptr2, a1 and a2 all seat on target and the only defect is the {out2, pa4, a3} 3-cycle (`out2=30 pa4=22 a3=23`). sandbox 17 at 132/132 insns. The remaining problem is now exactly one scalar: lift out2 from 714 into (1473, 1702) with no emitted byte.
- verdict: CONFIRMED — banked as `rejected/honest-vector-ifirst-out2-714-3cycle.c`.

## [s14] An in-loop1 definition of out2 can co-exist with target's slot-71 `addiu $s3,$s7,0x20`.
- mechanism: the source spelling of out3 was assumed to control the emitted form of out3's definition independently of where out2 is defined.
- probe: `W1_o2_loop1top` and `W2_o2_beforecall2` (out2 defined inside loop1, out3 spelled `(s32 *)((u8 *)pa4 + 0x20)`), `.lreg` tables plus a read of the post-flow RTL in `tmp/grind/func_80041188/s8/fd.flow`.
- result: KILLED, with the pass named. The `.flow` dump shows cse1 has already rewritten out3's definition to `(insn 164 (set (reg/v:SI 87) (reg/v:SI 86)))` — a plain `out3 = out2` copy — because with out2 defined inside loop1 the between-loops block sits in the SAME cse extended basic block as that definition, so cse1 knows `out2 == pa4 + 0x20` and substitutes the cheaper register. That substituted copy is also the source of the mysterious 4th reference s12 measured (4 refs / live 21 = 3809). Contrapositive, now proven: target's slot-71 insn REQUIRES out2's definition in block 0, a different EBB — which is exactly the region where s11 measured out2's live length capped at 43.
- verdict: KILLED

## s14 frontier (for s15)

1. **Requirement (B) is now a one-scalar problem with a named double bind, and the solver
   modality is the instrument for it.** out2 must reach a priority in (1473, 1702) — i.e.
   3 refs at live 17.6..20.4, or 4 refs at live 47.0..54.3 — while its DEFINITION stays in
   block 0 (E-s14-5: any other EBB placement makes cse1 rewrite out3's definition into the
   residual `move`). Mechanism: `tools/ra_solver` models global.c / local-alloc / reload
   exactly and `inverse_compose.py classify` converts a register-seat residual into a typed
   REACHABLE / FORECLOSED verdict with ranked C-lever vectors. Next probe: run
   `inverse_compose.py classify` against the `Z0_honest_ifirst` chassis (not candidate.c —
   Z0 is the form whose ONLY defect is this one seat) with stptr / i / tbl / stptr2 / a3
   pinned at their measured refs/live, goal = out2 into `$s6`; then check its lever vectors
   against the block-0-definition constraint from E-s14-5, which no previous solver run
   carried.

2. **Land the FAKE-free (A) fix and re-attack (B) from the `out3 = out2` chassis.**
   `alt_fakefree_floor3_s14.c` scores 3 with all-target seats and zero FAKE constructs; the
   shipping candidate scores 1 but carries an un-annotated F1 chain-extender that s11 proved
   would FAIL layer 1. Those are two different assets and the next session should decide
   which one is the base. Mechanism: the +2 gap between them is a sched1 INSN_LUID
   tie-break among three dependency-free between-block insns (E-s14-3), which is a
   `sched_solver` question, not an RA one. Next probe: run `tools/sched_solver` on the
   between-loops block of `X0_ipos` asking whether any dependency-preserving source order
   emits `addiu $s1`, `addiu $s2`, `li $s4,18` in target's order while keeping
   `i = 0x12;` at LUID position 1 — if it says no, the +2 is proven structural and the
   candidate must satisfy (A) some other way.

3. **Find the flow-counted / never-emitted construct target itself must contain.**
   E-s14-2 is a proof by measurement that target's honest reference vector cannot produce
   target's register assignment, so the original source contains at least one construct
   whose insns flow counts and which never reaches the bytes. The catalogue of passes that
   can delete an insn AFTER `flow_analysis` fixes `reg_n_refs` (toplev.c:2983) is short:
   combine (intra-block only — kills every between-block spelling, s8), reload's redundant
   copy elimination, and jump2 / cross-jump tail merging (which runs after allocation, so
   its deletions are free). s2's H2 tried the cross-jump route and failed only on
   byte-neutrality because sched1 interleaved the fall-in copy. Next probe: re-run the
   `duplicated-statement-into-arms` construction on the CURRENT `Z0_honest_ifirst` chassis
   rather than s2's long-dead split model, targeting out2 rather than tbl, and measure
   whether `jump2` re-merges the duplicated tails (read `.jump2` in the dump, not the
   sandbox score) — the two chassis differ by six sessions of structure and the s2 verdict
   is not chassis-valid.

## [s14] The residual is a SINGLE requirement on out2's allocno priority, as every session s7..s13 framed it (E-s12-3's band (1263,1702)).
- mechanism: global.c allocno_compare pri = floor_log2(reg_n_refs)*reg_n_refs/reg_live_length*10000; E-s12-3 enumerated out2's achievable (refs,live) pairs against a band bounded below by pa4 at 6 refs / 95 = 1263 and above by tbl at 4 refs / 47 = 1702.
- probe: Reproduced target's honest reference vector exactly (V1_honest_both = candidate.c with `stptr = base + 0xFC;` un-split AND `out3 = (s32 *)((u8 *)pa4 + 0x20);`), read the full .lreg priority table and .greg seat verdict via `bash tools/wsl.sh 'bash tmp/grind/func_80041188/s10/batch.sh ...'`, and compared the resulting rank order against target's seats in asm/funcs/func_80041188.s.
- result: FALSE as a single requirement. Target's seats require the strict order stptr > i > tbl > out2 > pa4 > a3, which is TWO conditions: (A) stptr > i and (B) tbl > out2 > pa4 > a3. The honest vector violates BOTH (i 2474 > stptr 2439; out2 714 < a3 808 < pa4 1473). The band in (B) is also (1473, 1702) not (1263, 1702): on the only chassis that emits target's slot-71 insn, pa4 carries out3's reference and measures 7 refs / 95 = 1473. E-s12-3's conclusion survives the correction (no achievable out2 pair lands in (1473,1702) either) but its arithmetic came from the wrong chassis.
- verdict: KILLED

## [s14] Requirement (A) — stptr > i — can be satisfied in ordinary C, retiring candidate.c's un-annotated F1 chain-extender `stptr = base; stptr += 0xFC;`.
- mechanism: flow.c:1685 counts one live insn per RTL insn a pseudo spans, and `i` is DEAD in the between-loops block between loop1's last read and the `i = 0x12;` redefinition. Moving the redefinition earlier shortens that dead gap, LENGTHENS reg_live_length(i), and therefore LOWERS i's priority floor_log2(8)*8/live*10000 — flipping (A) without touching stptr at all.
- probe: Four-position sweep of `i = 0x12;` through the between-loops block on the un-split-stptr chassis (X0_ipos..X3_ipos), each read for the full .lreg table plus the .greg seat verdict; X0_ipos then spliced into src/text1a_pre.c, scored with `sandbox func_80041188 --disable all`, and its residual slots extracted with a normalized objdump comparator (tmp/grind/func_80041188/s14/cmp.py).
- result: CONFIRMED. i measures 99/98/97/96 live (priority 2424 / 2448 / 2474 / 2500) at positions 1/2/3/4, and position 1 alone gives ALL-TARGET callee-saved seats with the chain-extender REMOVED — the first form in the ledger with all-target seats and zero FAKE constructs of any kind. sandbox 3 at 132/132 insns; residual is exactly `li $s4,18` emitted at slot 67 instead of slot 69 (pushing the two `addiu $s1/$s2,,0x6C` down one each) plus the long-known slot-71 `move $s3,$s6` vs `addiu $s3,$s7,0x20`.
- verdict: CONFIRMED

## [s14] The +2 emission-order cost of the `i = 0x12;`-first fix can be decoupled from its live-length win — by extending i's live range at the FRONT (earlier definition) or by shortening stptr's live range instead.
- mechanism: reg_live_length is computed by flow on PRE-sched RTL while emission order is decided by sched1's rank_for_schedule, which falls back to INSN_LUID for the three dependency-free between-block insns; both read the same source order, so decoupling needs a different variable than statement position in that block.
- probe: Y1_ifirst (`s32 i = 1;` declared first), Y2_ilate_init (`s32 i;` declaration split from an `i = 1;` statement at the head of block 0), Y3_stptr_first (stptr defined before saved/out2, to shorten stptr's live range from 41 to 40 and satisfy (A) from stptr's side). Full .lreg + .greg for each.
- result: KILLED, all three. Y1: i stays 8 refs / live 97 — the parameter copies emitted by expand_function_start always precede any user statement, so i's range cannot be extended at the front (only the allocno numbering changes: i becomes r77, pa4 r78). Y2: i 8 / 96 = 2500, worse. Y3: stptr 5 / 44 = 2272, worse — stptr's live range is pinned to loop1's insn count from below and lengthens, never shortens, when its definition moves earlier. The +2 is structural on this chassis.
- verdict: KILLED

## [s14] With requirement (A) satisfied honestly, the fully honest chassis reduces to requirement (B) alone.
- mechanism: direct measurement of the composed form (no new mechanism claim).
- probe: Z0_honest_ifirst = un-split stptr + `out3 = (s32 *)((u8 *)pa4 + 0x20)` + `i = 0x12;` first; .lreg table, .greg seats, and `sandbox func_80041188 --disable all` after splicing into src/text1a_pre.c.
- result: CONFIRMED. stptr 2439 > i 2424 > tbl 1702 > pa4 1473 > a3 808 > out2 714; stptr, i, tbl, stptr2, a1 and a2 all seat on target and the ONLY defect is the {out2, pa4, a3} 3-cycle (out2=$fp, pa4=$s6, a3=$s7). sandbox 17 at 132/132 insns. The remaining problem is now exactly one scalar: lift out2 from 714 into (1473, 1702) with no emitted byte.
- verdict: CONFIRMED

## [s14] An in-loop1 definition of out2 (the only shape reaching a short live range) can co-exist with target's slot-71 `addiu $s3,$s7,0x20`.
- mechanism: the source spelling of out3 was assumed to control the emitted form of out3's definition independently of where out2 is defined.
- probe: W1_o2_loop1top and W2_o2_beforecall2 (out2 defined inside loop1, out3 spelled `(s32 *)((u8 *)pa4 + 0x20)`), .lreg tables plus a direct read of the post-flow RTL in tmp/grind/func_80041188/s8/fd.flow.
- result: KILLED, with the pass named. The .flow dump shows cse1 has ALREADY rewritten out3's definition to `(insn 164 (set (reg/v:SI 87) (reg/v:SI 86)))` — a plain `out3 = out2` copy — because with out2 defined inside loop1 the between-loops block sits in the SAME cse extended basic block as that definition, so cse1 knows out2 == pa4 + 0x20 and substitutes the cheaper register. That substituted copy is also the source of the unexplained 4th reference s12 measured (4 refs / live 21 = 3809). Contrapositive, now proven: target's slot-71 insn REQUIRES out2's definition in block 0 (a different EBB), which is exactly the region where s11 measured out2's live length capped at 43.
- verdict: KILLED

## s15 (solver, 2026-08-24) — hypotheses and the frontier handed to s16

### Killed this session
- **H-s15-A (KILLED).** "The +2 emission-order gap on the honest `alt_fakefree`
  chassis is a sched1 tie-break that some other dependency-preserving source order
  can fix while keeping `i` live 99." `tools/sched_solver` enumerated the whole
  spellable atom space for block 2 (30 single atoms + all pairs) and returned a
  single vector, which IS the `i`-at-position-3 order; spelled → sandbox 15.
  Emission order and `reg_live_length(i)` are the same variable. (E-s15-1)
- **H-s15-B (KILLED).** "Loop 2 has its own counter in the original (`j`), which
  frees `i` from spanning both loops and solves requirement (A) for free."
  Measured sandbox 19; requirement (A) is indeed solved, but both counters fall
  below tbl/out2 and the chassis then needs a **3-atom** byte-free reference lift
  (i +1, j +1, out3 +2) versus P1's **1**. (E-s15-5)
- **H-s15-C (RETRACTED, not killed — s11/s12's claim).** "The out2 priority window
  (1473, 1702) is empty, therefore the residual is enumeration-complete." True only
  with tbl pinned at 1702. `inverse.py` returns two minimal vectors that lift tbl
  instead (E-s15-3), so the axis is open again — unspelled, but not closed.

### The frontier (ranked)

1. **Give `stptr` a sixth flow-counted reference, or end its live range one insn
   earlier, in ordinary C — on the P1 chassis.** P1
   (`memory/grind/func_80041188/alt_P1_honest_ipos3_s15.c`, sandbox 15, zero FAKE
   constructs, target's block-2 emission order already correct) reaches target's
   COMPLETE callee-saved disposition under that one perturbation and under nothing
   else (E-s15-4). Success = an honest floor 1 whose only residual is the
   long-standing slot-71 `out3` shape, i.e. it retires the un-annotated F1
   chain-extender that currently blocks candidate.c from ever landing.
   Mechanism: `global.c` allocno priority
   `floor_log2(n_refs)*n_refs/live_length*10000`; `stptr` is 5 refs / live 41 =
   2439 and needs to pass `i`'s 2474 — 6 refs gives 2926, live 40 gives 2500.
   Next probe: `stptr`'s live range starts at its block-0 definition (already the
   last statement of block 0 in source order) and ends at `stptr += 0x68` in loop
   1's delay slot, so attack the START: read the `.sched`/`.greg` dumps for P1
   (`pwsh tools/grinder/dump.ps1 func_80041188` after splicing P1) to see which
   block-0 insn sits between `addiu $s3,$v0,0xFC` and the block end, and whether
   any source-level reordering of `saved` / `out2` / `stptr` (or of the `saved`
   spill store `sw $t0,0x18($sp)`) moves the definition one slot later. Re-model
   with `bash tmp/grind/func_80041188/s15/remodel.sh` and read `allocdbg` — the
   model answers in seconds without a sandbox run. Note the sched_solver can be
   asked the same question directly for block 0 now that extraction works.

2. **Spell one of the two tbl-lift vectors on the Z0 chassis** (`tbl refs 4→5` or
   `tbl live 47→39`, each paired with `out2 refs 3→4`). This is the axis s11/s12
   believed closed. Mechanism: with tbl at 2127 (5 refs / 47) or 2051 (4 / 39),
   out2 may sit at 1904 (4 refs / live 42) instead of needing the unreachable
   47..55 live window, and Z0 is the chassis that already emits target's
   `addiu $s3,$s7,0x20`. Next probe: enumerate the honest 5th `tbl` reference —
   `*tbl` is read once and `tbl++` once in loop 1, and a re-read across the
   intervening `func_8004A348` call cannot be CSE'd (the call may clobber memory),
   so it is a real flow-counted reference; measure whether combine/cse delete it
   again before it materialises an insn, using the model rather than the sandbox.

3. **Vector #4: `pa4 refs 7→4` + `out2 live 42→34`** — the only minimal vector that
   needs no 4th `out2` reference at all, and therefore the only one that does not
   depend on the ledger's oldest unsolved sub-problem. Mechanism: `pa4` is 7 refs /
   95 = 1473 on Z0 and every reference is a real call argument in target's bytes,
   so the lift must come from re-routing three of them through a value that already
   exists (not from deleting them). Next probe: check in the `.flow` dump which of
   `pa4`'s 7 references are the `pa4 = a4` copy and its cse'd descendants — if the
   parameter `a4` and `pa4` are separate pseudos pre-regmove, a source form that
   uses `a4` directly for the loop-2 arguments may split the reference count
   without changing a byte.

## [s15] The +2 emission-order gap between the FAKE-free s14 form (floor 3) and candidate.c (floor 1) is a sched1 tie-break that some other dependency-preserving source order can fix while keeping reg_live_length(i) = 99.
- mechanism: sched.c rank_for_schedule falls back to INSN_LUID (= source order) for the three mutually independent between-block insns; flow's live-length bookkeeping reads the same order, so the two are coupled only if no order separates them.
- probe: tools/sched_solver extract (via the reduced-TU shim) + goalmap + perturb.py --atoms luid,luid_move --depth 2 with the target pinned to a tgt.head.s built from HEAD source; then spell the returned vector in C and measure.
- result: Block 2 (5 insns) differs in 3 slots; 30 single atoms + all pairs yield exactly ONE vector, 'luid_move 152 -> immediately before 161' = move `i = 0x12;` to sit immediately before `stptr2 = saved + 0x750;'. Spelled and measured: emission order becomes target's and the sandbox score goes 3 -> 15, because that same position sets reg_live_length(i) = 97 instead of 99 (i 2474 > stptr 2439).
- verdict: KILLED

## [s15] The original has a SEPARATE loop-2 counter (target re-initialises $s4 with `addiu $s4,$zero,0x12` between the loops), which frees `i` from spanning both loops and satisfies requirement (A) without any FAKE construct.
- mechanism: Two non-conflicting locals can share one hard register in GCC 2.7.2 (as out3/stptr already do here), and each counter's reg_live_length collapses from 97-99 to ~48, dropping both priorities below stptr's 2439.
- probe: Spelled `s32 j` for loop 2 on the P1 chassis, measured the sandbox, re-extracted the RA model, and forward-replayed global.c over all single-pseudo and all {i,j}-pair perturbations.
- result: sandbox 19 / 132 of 132. Requirement (A) IS solved (i 4 refs/48 = 1666, j 4/49 = 1632, both under stptr 2439) but both counters fall below tbl and out2 (1702) so four seats permute; zero single-pseudo and zero {i,j}-pair perturbations reach all-target seats, because j and out3 conflict across loop 2 and target wants the LOWER-priority out3 in the LOWER register $s3. The minimum that works is a 3-atom lift (i +1 ref, j +1 ref, out3 +2 refs) versus P1's 1 atom.
- verdict: KILLED

## [s15] s11/s12's enumeration-completeness claim - out2's priority must land inside (1473, 1702) and its live-length ceiling on the pa4-derived chassis is 43, therefore the residual is closed - holds.
- mechanism: global.c allocno priority floor_log2(n)*n/live*10000 with tbl's 1702 treated as a fixed upper bound.
- probe: tools/ra_solver/inverse.py global on the Z0 (out3 = pa4 + 0x20) chassis with the FULL target disposition as the goal, depth 2 and depth 3.
- result: RETRACTED. Minimal solution size 2, four vectors: (tbl refs 4->5 + out2 refs 3->4), (tbl live 47->39 + out2 refs 3->4), (out2 refs 3->4 + out2 live 42->50), (pa4 refs 7->4 + out2 live 42->34). The (1473,1702) window is empty only with tbl pinned at 1702: out2 at 4 refs/live 42 = 1904 works if tbl is lifted to 2127 (5 refs) or 2051 (live 39). Two of the four vectors, and the pa4-refs-down vector, were never considered by any prior session. All four remain unspelled.
- verdict: KILLED

## [s15] With the block-2 emission order already correct (P1), target's complete callee-saved disposition is far out of reach and needs a multi-atom restructure.
- mechanism: P1's measured allocno table has i at 2474 outranking stptr at 2439 by 35 priority units, shifting stptr/i/out3 one seat each.
- probe: Forward-replayed global.c on P1's extracted model over every +/-1..4 live-length and +/-1..2 reference perturbation of all ten contested pseudos (tmp/grind/func_80041188/s15/probe_ra.py).
- result: FALSE, and this is the session's load-bearing result: P1 reaches ALL-TARGET seats under a SINGLE atom, and only these - stptr live 41->40 (also 39/38/37), stptr refs 5->6 (also 7), or i live 97->99 (the s14 route that costs the +2 emission order). Nothing else in the model reaches it. The honest problem is now one unit of stptr live range or one honest stptr reference - exactly what candidate.c's un-annotated F1 chain-extender fakes.
- verdict: CONFIRMED

## [s16] `stptr` live 41 -> 40 (one of E-s15-4's three single atoms) is reachable by some statement placement or block-0 shape
- mechanism: global.c's allocno priority is `floor_log2(n_refs)*n_refs/live_length*10000`; at 5 references `pri = 100000/live`, so live 40 gives 2500 > i's 2474 and reaches target's complete disposition (s15 forward replay).
- probe: read `reg_live_length`'s true provenance out of the compiler source, then measure `stptr`'s per-basic-block live SEGMENTS with the instrumented cc1's `BB2_SLL_DEBUG` hook (`sched.c:3165`) on the P1 chassis.
- result: **KILLED.** `sched.c:5074-5106` overwrites `reg_live_length` with sched1's own per-block accounting, so the number the allocator sees is measured over the SCHEDULED insn list. `SLLDBG reg=88 seg=40 total=41`: block-0 segment 1, loop1 segment 40. stptr's defining insn is already the LAST insn of scheduled block 0 (RTL `insn 43` immediately precedes `code_label 44 "loop1"`), so the block-0 segment is at its floor, and stptr is loop-carried so its loop1 segment is the entire 40-insn block. Live 40 requires loop1 to hold one fewer insn at sched1 time — a byte change. The only untested loophole is turning one sched1-era loop1 insn into a `reload`-materialised one (loop1 emits 43 asm insns from 40 sched1 insns), which is not a C-level lever.
- verdict: KILLED — the P1 frontier collapses from three atoms to one (`stptr` refs 5 -> 6 or 7).

## [s16] An ORDINARY-C sixth flow-counted reference to `stptr` exists, i.e. the F1 chain-extender in candidate.c can be replaced by a construct that is not a FAKE
- mechanism: `reg_n_refs` is counted at flow, before combine, and never recomputed (`flow.c:2081`; `combine.c:55-56`), so any reference deleted by a later pass is still counted. combine is the only insn-deleting pass between flow and global_alloc.
- probe: enumerate the shapes that can put `stptr` in a sixth pre-combine insn and survive cse2 (which runs BEFORE flow and, per s8's cost-gate result, folds cheap chains away and lets flow delete them uncounted); measure each on P1 with the instrumented ALLOCDBG table.
- result: **NOT FOUND, and the search space is now characterised.** combine deletes an insn only by folding it into its consumer, so a sixth `stptr`-referencing insn must be a constant-offset chain step on `stptr` inside one basic block — i.e. structurally an F1 chain-extender. Three non-chain spellings were measured on sibling pseudos to test whether a *use-only* (+1) fold-back can dodge cse: `tb1 = tbl + 1; offset = tb1[-1]*6;` and `o2b = out2 + 4; call(o2b - 4)` both leave the pseudo's reference count untouched (cse2 folds the pair and flow deletes the dead insn uncounted), and `if (i - 0x12 < 0)` likewise leaves `i` at 8. So the use-only +1 lever does not exist in this compiler configuration; only the set+use chain step (+2) is known to survive, and only on some pseudos.
- verdict: OPEN, but reframed — see the next entry, which is the reason it matters less than it looked.

## [s16] Target's own source contained a `stptr` reference that its bytes do not show
- mechanism: live lengths are a function of the scheduled block layout (E-s16-1/E-s16-2). Target's block-0 and block-2 emissions are insn-for-insn identical in order to P1's, so target's `reg_live_length` values ARE ours: `stptr` 41, `i` 97. Target's `i` reference vector read off its bytes is 8, giving `pri(i) = 2474`; target seats `stptr` above `i`, so `pri(stptr) > 2474`, which at live 41 requires >= 6 references.
- probe: count `$s3`-referencing insns in target's loop1 (`asm/funcs/func_80041188.s`).
- result: **CONFIRMED.** Exactly FIVE: `addiu $s3,$v0,0xFC`, `addiu $a3,$s3,0x38`, `sh $v0,0x6($s3)`, `addiu $s3,$s3,0x68` (set+use). Five references at live 41 is 2439 < 2474. Target therefore carried at least one `stptr` reference that combine deleted.
- verdict: CONFIRMED — `candidate.c`'s `stptr = base; stptr += 0xFC;` reconstructs a construct the ORIGINAL demonstrably had. This is a target-derived argument for the F1 family on this function, not a search artefact, and it should be quoted verbatim in any FAKE annotation or ruling request.

## [s16] The F1 chain-extender shape is portable to `out2`, giving the 4th/5th reference the `out3 = pa4 + 0x20` chassis needs
- mechanism: the shape that lifts `stptr` 5 -> 7 is `x = y; x += c;` — a register copy followed by a constant add, folded by combine after flow has counted both insns. `out2 = pa4 + 0x20` has the identical algebraic shape, so `out2 = pa4; out2 += 0x20;` should lift out2 3 -> 5 refs (2381), which with a lifted `tbl` is one of s15's four Z0 vectors.
- probe: spell it on P1 and read the instrumented ALLOCDBG table (`rejected/out2-copy-then-modify-plus2-folds-preflow-unlike-stptr.c`).
- result: **KILLED.** `out2` stays at 4 refs / live 47 and the whole table is identical to plain P1 — cse2 folds the pair and flow deletes the copy uncounted. The character-for-character identical shape survives on `stptr` and dies on `out2`.
- verdict: KILLED, with a general lesson banked: F1 survival is a per-pseudo property of cse's cost gate, not of the syntax. No ref-lift may be predicted from a working sibling; each must be measured.

## [s16] stptr live 41 -> 40 (one of E-s15-4's three single atoms that reach target's complete disposition on the P1 chassis) is reachable by some statement placement or block-0 shape.
- mechanism: global.c:635-656 allocno_compare priority = floor_log2(n_refs)*n_refs/live_length*10000; at 5 refs pri = 100000/live, so live 40 = 2500 > i's 2474 and the s15 forward replay reaches ALL-TARGET seats.
- probe: Read reg_live_length's provenance out of the compiler source, then measure stptr's per-basic-block live SEGMENTS with the instrumented cc1's BB2_SLL_DEBUG hook (sched.c:3165) on the P1 chassis spliced into src/text1a_pre.c.
- result: sched.c:5074-5106 OVERWRITES reg_live_length with sched1's own per-block accumulation over the SCHEDULED insn list, so the allocator's number is post-sched1. SLLDBG reg=88 gives seg=40 total=41: block-0 segment 1, loop1 segment 40. The .flow RTL confirms stptr's def (insn 43) is already the LAST insn of scheduled block 0, immediately preceding code_label 44 'loop1', and stptr is loop-carried so its loop1 segment is the entire 40-insn block. Live 40 therefore requires loop1 to hold one fewer insn at sched1 time = a byte change.
- verdict: KILLED

## [s16] Target's own source contained a stptr reference that its bytes do not show, i.e. the F1 chain-extender in candidate.c reconstructs an original construct rather than inventing one.
- mechanism: Live lengths are a function of the scheduled block layout; target's block-0 and block-2 emissions are insn-for-insn identical in ORDER to P1's, so target's own reg_live_length values are ours (stptr 41, i 97) and its 8-reference i vector gives pri(i) = 2474. The s14 seat rule requires pri(stptr) > pri(i) for target's disposition, and at live 41 that needs >= 6 references (5 refs = 2439, 6 = 2926, 7 = 3414).
- probe: Count $s3-referencing insns in target's loop1 in asm/funcs/func_80041188.s and compare target's block-0/block-2 emission order against P1's dump output.
- result: Target's block 0 (asm/funcs/func_80041188.s:2-28) matches P1's position for position, differing only in register numbers, and target's block 2 emits i's redefinition 3rd exactly as P1 does. Target's loop1 contains exactly FIVE $s3-referencing insns (addiu $s3,$v0,0xFC; addiu $a3,$s3,0x38; sh $v0,0x6($s3); addiu $s3,$s3,0x68 counted as set+use). Five references at live 41 is 2439 < 2474, so the original source must have carried at least one further stptr reference that combine deleted.
- verdict: CONFIRMED

## [s16] The F1 chain-extender shape is portable to out2, giving the 4th/5th out2 reference that the out3 = pa4 + 0x20 chassis (the only one emitting target's slot-71 addiu $s3,$s7,0x20) needs.
- mechanism: The shape that lifts stptr 5 -> 7 refs is `x = y; x += c;` -- a register copy plus a constant add, folded by combine AFTER flow has counted both insns. out2 = pa4 + 0x20 is algebraically identical, so `out2 = pa4; out2 += 0x20;` should lift out2 3 -> 5 refs (2381), which is one half of s15's Z0 vectors.
- probe: Spell it on P1 and read the instrumented cc1 ALLOCDBG table (nrefs/livelen/pri per pseudo -- the exact inputs allocno_compare consumes) via tmp/grind/func_80041188/s16/model.sh.
- result: out2 stays at 4 refs / live 47 and the entire priority table is identical to plain P1: cse2 folds the pair and flow deletes the copy uncounted. The character-for-character identical shape survives on stptr and dies on out2, so F1 survival is a per-pseudo property of cse's cost gate, not of the syntax.
- verdict: KILLED

## [s16] A USE-ONLY (+1) combine-foldable reference exists -- a spelling that adds one flow-counted reference without the set+use of a chain step -- which would make the odd-numbered lifts (tbl 4->5, out2 3->4) in s15's four Z0 vectors spellable.
- mechanism: flow.c:2081 counts reg_n_refs pre-combine and combine.c never adjusts it, so an insn like `tb1 = tbl + 1` that combine folds back into its consumer is counted once for tbl and emits nothing.
- probe: Two spellings measured on P1 with the ALLOCDBG table: `tb1 = tbl + 1; offset = tb1[-1] * 6;` (tbl) and `o2b = (s32*)((u8*)out2 + 4); func_8004A348(buf, (s32*)((u8*)o2b - 4));` (out2). Plus a compare-split `if (i - 0x12 < 0)` aimed at i via combine's simplify_comparison.
- result: All three leave the reference count untouched (tbl 4, out2 unchanged, i 8). cse2 -- which runs BEFORE flow -- folds each pair back to the one-insn form and flow then deletes the dead insn uncounted, the same cost gate s8 recorded for split-init. The use-only +1 lever does not exist in this compiler configuration; only the set+use chain step (+2) is known to survive, and only on some pseudos.
- verdict: KILLED

## [s17] The reason `stptr = base; stptr += 0xFC;` survives cse while `out2 = pa4; out2 += 0x20;` folds is a NAMED cse.c decision, not a per-pseudo cost gate — and once named it predicts which pseudos admit an F1 lift at all.
- mechanism: cse1 canonicalises every pseudo occurrence through canon_reg (cse.c:2569), which returns qty_first_reg[reg_qty[regno]]. The quantity's canonical register is chosen in make_regs_eqv (cse.c:826-881): on `X = Y`, X displaces Y as canonical iff (uid_cuid[last_uid[X]] > cse_basic_block_end || uid_cuid[first_uid[X]] < cse_basic_block_start) && uid_cuid[last_uid[X]] > uid_cuid[last_uid[Y]] (cse.c:844-857). If X wins, the `X += K` insn keeps referring to X, the copy survives to flow and is counted (+2 refs), and combine folds it back for free; if Y wins, canon_reg rewrites the add to use Y, the copy is dead, and flow deletes it uncounted.
- probe: Read the .rtl and .cse sections for func_80041188 out of `-da` dumps of candidate.c and of s16/B1_out2copy.c side by side (tmp/grind/func_80041188/s17/{CAND,B1}/red.i.{rtl,cse}), then read make_regs_eqv and canon_reg out of tools/gcc-2.7.2/cse.c. Then FALSIFY the law in both directions with two new variants whose only change is the live range of the SOURCE operand.
- result: The RTL confirms the rewrite exactly — candidate.c's insns 43/46 are untouched by cse1, while B1's insn 43 is rewritten from `(plus (reg 86) 32)` to `(plus (reg 77) 32)`. Both falsification tests came out as predicted: V3 (`saved = base + 0x94;` moved into the between-loops block, so base outlives stptr; the chain-extender itself untouched) drops stptr 7 refs/3414 -> 5 refs/2439; V1 (out2 chained off the raw `a4` param pseudo with `pa4 = a4` moved after it, so out2 outlives its source) lifts out2 4 refs/47 -> 6 refs/51, the first out2 reference-lift ever measured on this function.
- verdict: CONFIRMED. Supersedes s16's "F1 survival is a per-pseudo property of cse's cost gate" — no cost gate is involved and the property is computable from the source: X = Y; X += K lifts reg_n_refs(X) by 2 iff X's last use is after Y's last use.

## [s17] Some byte-free spelling exists that gives `out2` a 4th reference on the `out3 = pa4 + 0x20` chassis (the only chassis emitting target's slot-71 `addiu $s3,$s7,0x20`).
- mechanism: By the s17 law, an F1 lift on out2 requires a source pseudo whose last use precedes out2's. out2's value is a4 + 0x20, so the only candidate sources are pa4 or a second copy of the raw a4 param.
- probe: pa4 was measured in s16 (B1: folds — pa4 lives through loop2). This session measured the only other source: V1_out2_from_a4_chain.c, which reorders block 0 so that `out2 = a4; out2 += 0x20;` precedes `pa4 = a4`, making a4's pseudo die first.
- result: The lift happens (out2 6 refs / live 51) but the build emits 143 insns instead of 142. The extra insn is `move $23,$2`: once a4's pseudo has a second use, the `pa4 = a4` alias stops coalescing into the stack load. Target's block 0 (asm/funcs/func_80041188.s:2-28) emits `lw $s7,0x58($sp)` immediately followed by `addiu $s6,$s7,0x20`, i.e. target's a4 and pa4 are the SAME register — the only spelling that lifts out2 is the one target's bytes forbid.
- verdict: KILLED — and killed by derivation over the complete source set, not by sampling. There is no byte-free F1 reference-lift for out2 on this function.

## [s17] (s16 frontier 3) Three of loop1's emitted insns are created after sched1 by reload, so a source form that pushed a fourth insn into that class would drop stptr's live length 41 -> 40 with the bytes unchanged.
- mechanism: sched1 accumulates reg_live_length over its own scheduled insn list (sched.c:3165/5106), which does not yet contain reload-generated insns, while the scorer sees the emitted count.
- probe: Count RTL insns between `code_label loop1` and `code_label loop2` in the `-da` dumps of candidate.c at .combine, .lreg (post-sched1) and .greg (post-reload), diff the insn uids, and compare against the emitted asm.
- result: 45 / 45 / 46, emitted 46. Reload creates exactly ONE insn in the whole region — uid 300, `(set (reg t0) (mem (plus (reg sp) 24)))`, the reload of the SPILLED pseudo 85 (`saved`, ALLOCDBG hardreg=-1) feeding `stptr2 = saved + 0x750` — and it lands in the between-loops block, not in loop1. loop1 proper gains zero insns at reload. The "three insns" premise was a mismatch between BB2_SLL_DEBUG's per-block accounting (40) and the RTL insn count (45), not a real gap.
- verdict: KILLED. With E-s16-2 the live-length side of requirement (A) is now closed in full: stptr live 41 is invariant under every source form that preserves loop1's bytes.

## [s17] The reason `stptr = base; stptr += 0xFC;` survives cse1 and lifts reg_n_refs 5->7 while the character-identical `out2 = pa4; out2 += 0x20;` folds away is a specific, nameable cse.c decision rather than s16's claimed 'per-pseudo property of cse's cost gate', and once named it predicts which pseudos admit an F1 reference-lift at all.
- mechanism: cse1 canonicalises every pseudo occurrence through canon_reg (cse.c:2569), which returns qty_first_reg[reg_qty[regno]]. The quantity's canonical register is chosen in make_regs_eqv (cse.c:826-881): on `X = Y`, X displaces Y as canonical iff (uid_cuid[last_uid[X]] > cse_basic_block_end || uid_cuid[first_uid[X]] < cse_basic_block_start) && uid_cuid[last_uid[X]] > uid_cuid[last_uid[Y]] (cse.c:844-857). If X wins, the `X += K` insn keeps referring to X, the copy reaches flow and is counted (+2 refs, flow.c:2081) and combine later folds it back for free; if Y wins, canon_reg rewrites the add to use Y, the copy is dead and flow deletes it uncounted.
- probe: Read the func_80041188 sections of the .rtl and .cse dumps for candidate.c and for s16/B1_out2copy.c side by side (tmp/grind/func_80041188/s17/{CAND,B1}/red.i.{rtl,cse}); read make_regs_eqv and canon_reg out of tools/gcc-2.7.2/cse.c; then falsify the law in both directions with two variants whose only change is the live range of the chain's SOURCE operand.
- result: The RTL confirms the rewrite exactly: candidate.c's insns 43/46 are untouched by cse1, while B1's insn 43 is rewritten from `(plus (reg 86) 32)` to `(plus (reg 77) 32)`. Both falsification tests came out as predicted. V3 (`saved = base + 0x94;` moved into the between-loops block so `base` outlives `stptr`; the chain-extender itself untouched) drops stptr from 7 refs / pri 3414 to 5 refs / pri 2439 — exactly the two references the F1 construct exists to buy. V1 (out2 chained off the raw `a4` param pseudo with `pa4 = a4` moved after it, so out2 outlives its source) lifts out2 from 4 refs / live 47 to 6 refs / live 51 — the first out2 reference-lift ever measured on this function.
- verdict: CONFIRMED

## [s17] Some byte-free spelling exists that gives `out2` a 4th reference on the `out3 = pa4 + 0x20` chassis — the only chassis that emits target's slot-71 `addiu $s3,$s7,0x20`.
- mechanism: By the s17 law an F1 lift on out2 requires a chain source whose last use precedes out2's. out2's value is a4 + 0x20, so the complete source set is {pa4, a second copy of the raw a4 param}.
- probe: pa4 was measured in s16 (B1: folds, since pa4 lives through loop2). This session measured the only other member: V1_out2_from_a4_chain.c reorders block 0 so `out2 = a4; out2 += 0x20;` precedes `pa4 = a4`, making a4's pseudo die first.
- result: The lift happens (out2 6 refs / live 51) but the build emits 143 insns instead of 142: the extra insn is `move $23,$2` in block 0, because once a4's pseudo has a second use the `pa4 = a4` alias stops coalescing into the stack load. Target's block 0 (asm/funcs/func_80041188.s:2-28) emits `lw $s7,0x58($sp)` immediately followed by `addiu $s6,$s7,0x20`, i.e. in target a4 and pa4 ARE the same register. The only spelling that lifts out2 is the one spelling target's bytes forbid.
- verdict: KILLED

## [s17] (s16 frontier item 3) Three of loop1's emitted insns are created after sched1 by reload, so a source form that pushed a fourth insn into that class would drop stptr's live length 41 -> 40 with the bytes unchanged — the last surviving loophole in the live-40 kill.
- mechanism: sched1 accumulates reg_live_length over its own scheduled insn list (sched.c:3165 / 5106), which does not yet contain reload-generated insns, while the scorer sees the emitted count.
- probe: Count RTL insns between `code_label loop1` and `code_label loop2` in candidate.c's -da dumps at .combine, .lreg (post-sched1) and .greg (post-reload), diff the insn uids to identify what reload adds, and compare against the emitted asm.
- result: 45 / 45 / 46, emitted 46. Reload creates exactly ONE insn in the whole region: uid 300, `(set (reg t0) (mem (plus (reg sp) 24)))`, the reload of the SPILLED pseudo 85 (`saved`, ALLOCDBG hardreg=-1) feeding `stptr2 = saved + 0x750` — and it lands in the between-loops block, not in loop1. loop1 proper gains zero insns at reload; its emitted count equals its post-sched1 count. The 'three insns' premise was a mismatch between BB2_SLL_DEBUG's per-block accounting (40) and the RTL insn count (45), not a real gap.
- verdict: KILLED

## s18 (rederive, 2026-08-24)

### Killed this session

- **H-s18-a — "the array-base reading gives an honest `stptr` reference lift."** The
  layout rederivation (E-s18-1) says loop1's pointer is `&arr[1]` with `arr = base +
  0x94`, so the natural spellings are `stptr = saved + 0x68;` or `saved = stptr - 0x68;`.
  BOTH are inert: cse1 reassociates any plus-derivation inside the definition's EBB, even
  when the derived value is real and consumed in a later block (E-s18-2). KILLED.
- **H-s18-b — carried frontier item 1: "another block-0 pseudo may be a chain donor for
  `stptr`."** Enumerated and closed: `saved` is measured dead (it outlives `stptr`, so
  s17's make_regs_eqv law folds the copy), and the `D_800A9A10 + a0*4` / `a0` pseudos hold
  values a LOAD away from `base`, so no constant-add chain exists. `base` is the only
  donor. KILLED (E-s18-2).
- **H-s18-c — carried frontier item 3, upgraded from sampling to derivation.** Reference
  deltas ARE quantised to +2 in the no-note regime, and the reason is that `combine` can
  only delete an insn by merging it into a same-block consumer, which never increases the
  merged pseudo's pre-combine count unless the insn both sets and uses it (E-s18-3).
  CONFIRMED — and with it, all four of s15's minimal Z0 vectors are unreachable in the
  no-note regime.
- **H-s18-d — "a genuine `do { } while (i < 0x12)` loop1 buys the weighting honestly."**
  It does buy ALL-TARGET seats at 132 insns (sandbox 5, the best FAKE-free score on
  record), but `loop.c`'s strength reduction rewrites the biv (`base + 0x134`,
  `sh -50($19)`), a 5-insn residual that no seat change can remove. KILLED as a route to
  zero; kept as the proof that the weighting regime is real (E-s18-6).

### CONFIRMED this session — the new lever

- **H-s18-e — `do { } while (0);` weights every reference inside it by +1
  (flow.c:2081 `reg_n_refs += loop_depth`).** This is the only construct that produces an
  odd reference delta, and it makes s15's Z0 inverse vector #1 (`tbl 4->5` AND
  `out2 3->4`) spellable. Measured on form V7: the predicted ALLOCDBG table exactly, and
  **ALL-TARGET seats together with target's block-2 `addiu $s3,$s7,0x20`** — the first
  form in 18 sessions with both (E-s18-4). Sandbox 8 at 133 insns; the U4 variant is 6.

### Live frontier for s19

1. **Close V7's single extra insn.** The residual is one load-delay `nop`: with the out2
   wrap present the scheduler hoists `offset = offset + (s32) a2;` above the first `jal`
   (target keeps it after), so `i++` no longer fills the `lhu $v0,0($s0)` delay slot.
   Pulling `offset += a2` inside the wrap fixes the count (U6, 132 insns) but adds an `a2`
   reference and breaks the `a1`/`a2` tie. Probes, in order of cheapness: (a) keep the
   wrap boundary but change WHICH out2 reference is weighted — wrap
   `func_800523E0(pa4, out2, a3, stptr + 0x38);` instead and compensate the `pa4` +1
   (7 -> 8 = 2526, which must stay below `i`, so pair it with a wrap that lifts `i`);
   (b) spell the a2 half so the add is not an `a2` reference (e.g. a second walking
   pointer local for the a2 row, so the wrap contains the pointer's own reference instead
   of `a2`'s); (c) re-order the a2-half statements so the hoist target is occupied.
2. **Re-run `tools/ra_solver/inverse.py` on the Z0 table with ODD deltas re-enabled.**
   s15's vector enumeration is still valid arithmetic, but its verdicts were filtered by
   the (now-superseded) belief that +1 lifts are unspellable. Vectors #1/#2/#3 are all
   live again under wrap weighting, and #2 (`tbl live 47->39` + `out2 3->4`) has never
   been spelled. Ask the solver for the vector with the SMALLEST number of weighted
   references, since each wrap is a scheduling risk (E-s18-5).
3. **Use the layout reading (E-s18-1) for statement geometry, not for reference lifts.**
   `i` is the entry index and `stptr`'s exit value equals `stptr2`'s entry value, so
   spellings like `stptr2 = stptr;` are semantically exact (they cost `saved` entirely and
   change the insn count, so they are not byte-candidates) — but the reading is what
   justifies which statement orders are natural when choosing among equal-scoring forms,
   and it should be quoted in any future ruling-request about `saved`.

## [s18] The three block-0/block-2 constants are one 0x68-stride array based at base+0x94 (saved = &entries[0], stptr = &entries[1], stptr2 = &entries[18], i is the index, and loop1's exit pointer 0x7E4 equals stptr2's entry value), so the natural array-relative spellings of stptr should be the original's shape.
- mechanism: 0x94+0x68 == 0xFC, 0x94+0x750 == 0x7E4 == 0xFC + 17*0x68, and 0x750 == 18*0x68; combine cannot fold saved's def into base+0x7E4 because the two insns are in different basic blocks, which is why target computes base+0x94 in block 0 and spills it.
- probe: Spelled three array-relative forms on the candidate chassis and read the instrumented-cc1 ALLOCDBG table for each (s17/dump17.sh V1a/V2a/V3a): stptr = saved + 0x68; stptr = saved, stptr += 0x68; and stptr = base + 0xFC with saved = stptr - 0x68.
- result: All three leave stptr at 5 references (2439 / 2380): cse1 reassociates (plus (plus base 252) -104) into base+148 and the reference vanishes before flow. The layout reading is correct arithmetic but yields no reference lift.
- verdict: KILLED

## [s18] Carried frontier item 1: some OTHER block-0 pseudo dying before loop1 ends can donate an s17-law copy-chain to stptr, giving a differently-shaped alternative to the FAKE F1 construct.
- mechanism: make_regs_eqv (cse.c:844-857) keeps the defined register canonical only when its last use is later than the source's, so any block-0 pseudo dying inside block 0 qualifies as a donor and combine folds the pair back for zero bytes.
- probe: Enumerated the block-0 pseudos and measured the only non-trivial candidate: chain sourced from `saved` (V2a). The D_800A9A10+a0*4 address regs and a0 were closed by derivation (their values are a LOAD, not a constant add, away from base).
- result: The `saved` chain folds (saved outlives stptr — the law's own prediction, now validated a third time); no other donor exists. `base` is the unique donor, i.e. the current F1 chain-extender has no alternative spelling on this chassis.
- verdict: KILLED

## [s18] Carried frontier item 3: byte-free reference deltas are quantised to +2, so every s15 Z0 inverse vector that needs an odd delta (tbl 4->5, out2 3->4) is unreachable in principle.
- mechanism: flow counts references before combine; combine deletes an insn only by merging it into a same-basic-block consumer, and merging A (`q = R + k`) into B (`... q ...`) leaves the pair mentioning R exactly as often as B alone would. Only an insn that both SETS and USES the pseudo (a copy-chain) is counted twice by flow and then folded away.
- probe: Derived the rule from combine's merge algebra and cross-checked it against every measured spelling in the ledger (s16 E-s16-4's four, plus this session's V1a/V2a/V3a).
- result: CONFIRMED for the no-loop-note regime — which closes the Z0 chassis under that regime — but the derivation is a statement about combine, NOT about flow's counting rule, and that gap is what hypothesis 4 exploits.
- verdict: CONFIRMED

## [s18] A `do { ... } while (0);` wrap gives every reference inside it +1, which is the missing odd-delta dial and makes s15's Z0 vector #1 (tbl 4->5 AND out2 3->4) spellable.
- mechanism: flow.c:2081 is `reg_n_refs[regno] += loop_depth;` and loop_depth is the NOTE nesting depth (flow.c:440-471 / 1385-1449; flow.c:1453 aborts at 0, so the no-note baseline is 1). A do-while(0) emits LOOP_BEG/END notes with no back edge, so flow double-counts the enclosed references while loop.c performs no giv rewrite.
- probe: Form V7 (three wraps: the tbl load, func_8004A348(buf,out2), and *(s16*)(stptr+6) = 2), honest un-split stptr = base + 0xFC, Z0 block 2 (out3 = pa4 + 0x20): read the ALLOCDBG table, then sandboxed it and six placement variants (T1..T4, U1, U4..U8).
- result: The table matched the prediction exactly and every callee-saved seat is target's (stptr 6/41=2926 $s3, stptr2 6/48=2500 $s0, i 8/97=2474 $s4, tbl 5/47=2127 $s5, out2 4/42=1904 $s6, pa4 7/95=1473 $s7, a3 4/99=808 $fp, out3 $s3, saved spilled) WHILE block 2 emits target's addiu $s3,$s7,0x20 — the E-s16-5 lock is broken. Sandbox 8 at 132 target / 133 build insns; the U4 variant scores 6. The single extra insn is a load-delay nop.
- verdict: CONFIRMED

## [s18] A genuine `do { } while (i < 0x12)` loop1 buys the same weighting honestly (no wrap, no FAKE construct at all).
- mechanism: A real loop note raises loop_depth to 2 inside loop1, so every in-loop reference counts twice — stptr 9 refs, i 11, tbl 7, out2 6, pa4 8.
- probe: Form V5 (real do-while loop1, honest un-split stptr, out3 = out2): ALLOCDBG table + sandbox; and V6, its Z0 variant.
- result: V5 reaches ALL-TARGET seats at 132 insns and sandbox 5 — the best score ever recorded on a wholly FAKE-free form — but loop.c's strength reduction rewrites the biv (base+0x134, `move $7,$19`, `sh $8,-50($19)` vs target's addiu $a3,$s3,0x38 / sh $v0,0x6($s3)); that 5-insn residual is not a seat question. V6 (Z0 block 2) is not all-target: weighting lifts pa4 to 9/94=2872 above out2 5/41=2439 and the two swap.
- verdict: KILLED

## [s19] s18's V5 residual is a giv/biv-elimination artefact, and writing the memory layout as an INDEXED array (i as the biv, address inline) removes it — giving a FAKE-free form whose walking-pointer priority is honest.
- mechanism: with an explicit walking pointer, `stptr` is itself the biv and loop.c eliminates it in favour of the `stptr + 0x38` giv (base+0x134, `sh $8,-50($19)`). With `i` as the biv the biv cannot be eliminated (it feeds the exit test and loop2), so loop.c creates ONE address giv and expresses both uses as offsets from it — exactly target's `addiu $a3,$s3,0x38` + `sh $v0,0x6($s3)`.
- probe: form W4 (memory/grind/func_80041188/alt_W4_realloop_giv_honest_s19.c) — loop1 a real `do {} while (i < 0x12)`, address written inline as `ents + i * 0x68 (+0x38 / +6)`, no pointer local; ALLOCDBG table + sandbox + objdump differ (tmp/grind/func_80041188/s19/odiff.py).
- result: CONFIRMED. sandbox 3 at 132 target / 132 build insns with ALL-TARGET callee-saved seats and ZERO FAKE constructs; the giv allocno is 9 refs / live 40 = 6750, which is the free, honest replacement for candidate.c's FAKE F1 chain-extender. The 3 residual insns are 2 (real-loop constant tax, next entry) + 1 (the standing out3 lock).
- verdict: CONFIRMED

## [s19] The induction address may be given a name (`ent = ents + i * 0x68;`) without cost.
- mechanism (expected): loop.c renames the giv's destination, so a user variable and a compiler pseudo should be interchangeable.
- probe: form W1, identical to W4 except the address is a local; ALLOCDBG + sandbox + red.i.combine.
- result: KILLED, sandbox 80. loop.c does NOT rename a REG_USERVAR_P destination — it emits `ent = <giv>` at the top of every iteration (insn 339). That copy is a second quantity live across the calls; local_alloc seats it in $s1, the nine callee-saved seats fill before a3 is reached, and a3 spills. Never name an induction address in this project.
- verdict: KILLED

## [s19] The 2-insn constant residual of every real-loop form (`addiu $t0,zero,2` / `sh $t0,6($s3)` vs target's $v0) is removable at C level.
- mechanism (tested): loop.c hoists the invariant `li 2` out of loop1, the hoisted pseudo becomes a whole-function allocno (3 refs / live 92), global.c spills it, and reload rematerialises the constant into $t0 — where target's in-place `li` is a local quantity local_alloc gives $v0. The C-level escape would be to make scan_loop reject the movable.
- probe: read scan_loop's movable test (loop.c:686-700) and move_movables' profitability test (loop.c:1631) against this loop's parameters, and cross-checked the decomposition against s18's V5 score (2 giv + 2 tax + 1 lock = 5) and W4's (0 + 2 + 1 = 3).
- result: KILLED BY DERIVATION. The three movable conditions are OR'd; a single-basic-block loop body satisfies (3) `reg_in_basic_block_p` unconditionally, so making the value a user variable (which defeats (2)) changes nothing, and (1) also passes because the store is executed on every iteration. The profitability test passes for any lifetime >= 1 since `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` ~ 31 against insn_count ~ 40. Every real-loop spelling of loop1 therefore starts 2 insns behind candidate.c's goto chassis — this, not the seats, is what caps the whole real-loop family at floor 2.
- verdict: KILLED

## [s19] On the real-loop chassis with target's block-2 spelling (Z0), out2 needs exactly ONE more flow-counted reference and nothing else.
- mechanism: measured priorities on W6 put pa4 at 9 refs / live 94 = 2872 and tbl at 7/47 = 2978 with every other seat already target's, so out2 must land in the open interval (2872, 2978); floor_log2 quantisation admits exactly one solution, 6 refs at live 41 = 2926, and out2's live length on this chassis is already 41.
- probe: W6 (Z0, honest) ALLOCDBG + sandbox; then W8 = W6 + one in-loop `do { func_8004A348(buf, out2); } while (0);` to synthesise the missing reference.
- result: CONFIRMED by construction. W8's table is the predicted one and it is the FIRST form to hold ALL-TARGET seats AND emit target's `addiu $s3,$s7,0x20` on the real-loop chassis; its only cost is three maspsx load-delay nops (135 insns) caused by the extra LOOP notes inside the loop body. The requirement is therefore exactly: +1 reference on out2, byte-free, without adding LOOP notes inside loop1.
- verdict: CONFIRMED

## [s19] A do-while(0) wrap placed in BLOCK 0 (outside loop1) can supply that +1 for free.
- mechanism: flow.c:2081's `+= loop_depth` applies wherever the notes are, and block 0 is not scheduled against the loop body, so a block-0 wrap should be byte-neutral.
- probe: W10 = W6 + `o2t = (u8 *)pa4 + 0x20; do { out2 = (s32 *)o2t; } while (0);`, with W11 (same split, no wrap) as the control.
- result: HALF-CONFIRMED, HALF-KILLED. The wrap IS byte-neutral (132 build insns, versus 135 for the in-loop wrap) — a reusable fact. But it lifted nothing: out2 stayed at 5 refs, because the wrapped insn is a copy whose destination outlives its source, so make_regs_eqv (cse.c:844-857) keeps out2 canonical and cse1 deletes the copy before flow counts it (W11 measures identical to W6). A block-0 wrap must enclose a reference that SURVIVES cse1; by the s17 law that means the donor must outlive the recipient, and out2's only block-0 donor is pa4 — whose reference the same wrap would also weight, pushing pa4 to 10 refs / 3191, back above out2.
- verdict: KILLED (as spelled); the byte-neutrality of block-0 wraps is CONFIRMED and carried.

## [s19] s18's V5 residual is a giv/biv-elimination artefact, and writing the memory layout as an INDEXED array (i as the biv, the entry address written inline at its use sites) removes it, giving a FAKE-free form whose walking-pointer priority is honest.
- mechanism: With an explicit walking pointer, stptr is itself the biv and loop.c eliminates it in favour of the stptr+0x38 giv (base+0x134, sh $8,-50($19)). With i as the biv the biv cannot be eliminated (it feeds the exit test and loop2), so loop.c creates ONE address giv and expresses both uses as offsets from it -- target's addiu $a3,$s3,0x38 + sh $v0,0x6($s3).
- probe: Form W4 (memory/grind/func_80041188/alt_W4_realloop_giv_honest_s19.c): loop1 a real do{}while(i<0x12), address inline as ents + i*0x68 (+0x38/+6), no pointer local. ALLOCDBG table via s19/dump19.sh + sandbox + objdump differ (s19/odiff.py).
- result: sandbox 3 at 132 target / 132 build insns, ALL-TARGET callee-saved seats, ZERO FAKE constructs; giv allocno 9 refs / live 40 = 6750 (candidate.c has to buy stptr 7 refs with the FAKE F1 chain-extender). Loop2's init comes out as ents + 0x750 = target's lw $t0,0x18($sp) + addiu $s0,$t0,0x750, so the block-0 spill is explained by the layout.
- verdict: CONFIRMED

## [s19] The induction address may be given a name (ent = ents + i * 0x68;) without cost.
- mechanism: Expected loop.c to rename the giv's destination, making a user variable and a compiler pseudo interchangeable.
- probe: Form W1 (identical to W4 except the address is a local); ALLOCDBG + sandbox + red.i.combine insn 339.
- result: sandbox 80. loop.c does NOT rename a REG_USERVAR_P destination: it emits `ent = <giv>` at the top of every iteration. That copy is a second quantity live across the calls, local_alloc seats it in $s1, the nine callee-saved seats fill before a3 is reached, and a3 SPILLS. Banked as rejected/realloop-giv-uservar-copy-steals-s1-a3-spills.c.
- verdict: KILLED

## [s19] The 2-insn constant residual of every real-loop form (addiu $t0,zero,2 / sh $t0,6($s3) where target has $v0) is removable at C level.
- mechanism: loop.c hoists the invariant `li 2` out of loop1; the hoisted pseudo becomes a whole-function allocno (3 refs / live 92), global.c spills it, and reload rematerialises the constant into $t0, where target's in-place li is a local quantity local_alloc gives $v0.
- probe: Read scan_loop's movable test (loop.c:686-700) and move_movables' profitability test (loop.c:1631) against this loop's parameters; cross-checked the decomposition against s18's V5 score (2 giv + 2 tax + 1 lock = 5) and W4's (0 + 2 + 1 = 3).
- result: KILLED BY DERIVATION. The three movable conditions are OR'd and a single-basic-block loop body satisfies (3) reg_in_basic_block_p unconditionally, so making the value a user variable (which defeats (2)) changes nothing, and (1) also passes because the store executes every iteration. Profitability passes for any lifetime >= 1 since threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) ~ 31 against insn_count ~ 40. Every real-loop spelling of loop1 therefore starts 2 insns behind the goto chassis; this caps the whole real-loop family at floor 2.
- verdict: KILLED

## [s19] On the real-loop chassis with target's own block-2 spelling (out3 = (s32 *)((u8 *)pa4 + 0x20)), out2 needs exactly ONE more flow-counted reference and nothing else.
- mechanism: Measured priorities put pa4 at 9 refs / live 94 = 2872 and tbl at 7/47 = 2978 with every other seat already target's, so out2 must land in the open interval (2872, 2978); floor_log2 quantisation admits exactly one solution, 6 refs at live 41 = 2926, and out2's live length on this chassis is already 41.
- probe: W6 (Z0, honest) ALLOCDBG + sandbox; then W8 = W6 + one in-loop do { func_8004A348(buf, out2); } while (0); to synthesise the missing reference.
- result: CONFIRMED by construction. W8's table is the predicted one (out2 6/41 = 2926 -> $s6, pa4 $s7, tbl $s5, i $s4, giv $s3, stptr2 $s0, a3 $fp, out3 $s3) and it is the first form on this chassis to hold ALL-TARGET seats AND emit target's addiu $s3,$s7,0x20; its only cost is three maspsx load-delay nops (135 insns) caused by the extra LOOP notes inside the loop body.
- verdict: CONFIRMED

## [s19] A do-while(0) wrap placed in BLOCK 0 (outside loop1) can supply that +1 reference for free.
- mechanism: flow.c:2081's += loop_depth applies wherever the notes are, and block 0 is not scheduled against the loop body, so a block-0 wrap should be byte-neutral.
- probe: W10 = W6 + `o2t = (u8 *)pa4 + 0x20; do { out2 = (s32 *)o2t; } while (0);`, with W11 (same split, no wrap) as the control.
- result: Byte-neutrality CONFIRMED (132 build insns vs 135 for the in-loop wrap) but the lift is KILLED: out2 stayed at 5 refs because the wrapped insn is a copy whose destination outlives its source, so make_regs_eqv (cse.c:844-857) keeps out2 canonical and cse1 deletes the copy before flow counts it (W11 measures identical to W6). A block-0 wrap must enclose a reference that survives cse1; out2's only block-0 donor is pa4, whose reference the same wrap would also weight (pa4 -> 10 refs / 3191, back above out2).
- verdict: KILLED

## [s20] A do-while(0) wrap in BLOCK 0 can supply out2's missing flow reference on the GOTO chassis (s19 concluded block-0 wraps weight nothing).
- mechanism: flow.c:2081 (`reg_n_refs += loop_depth`) counts every reference in an insn enclosed by LOOP_BEG/END notes twice. s19's W10 wrapped a redundant COPY, which cse1 deletes before flow; out2's DEFINITION is a plus, which cse1 cannot delete, so it should be counted at depth 2.
- probe: Form Y1 = s18 V7 with the costly out2-CALL wrap replaced by `do { out2 = (s32 *)(((u8 *)pa4) + 0x20); } while (0);` in block 0; ALLOCDBG via s20/dump20.sh + sandbox.
- result: The lift is REAL and free — out2 3 -> 4 refs at live 42, priority 714 -> 1904 (exactly inside the required (pa4, tbl) window), 132 build insns. But the same wrap co-weights pa4 (7 -> 8 refs, 1473 -> 2526), which jumps pa4 above out2, tbl AND i: sandbox 27.
- verdict: CONFIRMED as a mechanism (correcting E-s19-6's scope), KILLED as a route — out2's only block-0 insn references pa4 by construction.

## [s20] Sourcing the wrapped definition from the parameter name (`a4`) instead of the local copy keeps the wrap's second +1 off pa4.
- mechanism: If a4 and pa4 are distinct pseudos, the wrapped insn references a4, leaving pa4 at its natural 7 refs.
- probe: Y2a (declaration `s32 *pa4 = a4;` kept, wrapped def written from a4) vs Y2b (declaration demoted to `s32 *pa4;`, `pa4 = a4;` written as a statement AFTER the wrapped def).
- result: Y2a is INERT — its ALLOCDBG table is byte-identical to Y1 (pa4 8/95 = 2526, sandbox 27), because cse1 canonicalises a4 to pa4 inside block 0's EBB (pa4 outlives a4; s17 make_regs_eqv law). Y2b DOES keep them distinct and delivers ALL-TARGET seats together with target's block-2 `addiu $s3,$s7,0x20` — the first time that combination has been reached on the goto chassis (out2 4/43=1860 $s6, pa4 6/93=1290 $s7, stptr $s3, i $s4, tbl $s5, a3 $fp, stptr2 $s0) — but at 133 build insns, sandbox 12. The split makes a4 a block-0-LOCAL quantity, so local_alloc seats it in $v0 before global_alloc sees pa4, and `addu $s7,$v0,$zero` is emitted where target has `lw $s7,0x58($sp)` straight into the seat. Y4 (same split, stptr via the F1 chain instead of a wrap) measures identically at 12/133.
- verdict: KILLED as spelled (the copy is structural — GCC 2.7 does not coalesce a local quantity into a global allocno), but the AXIS is alive: any spelling that keeps the two reference sets apart without materialising the copy is distance 0.

## [s20] A semantically real back-derivation (`pa4 = (s32 *)((u8 *)out2 - 0x20);`) gives out2 a fourth reference without any wrap.
- mechanism: E-s18-2 killed forward plus-derivations inside the definition's EBB; the reverse direction (deriving the SOURCE pointer from the derived one) puts the surviving reference on out2 instead of pa4.
- probe: Z1, with Z2 (`pa4 = a4;` written plainly) as the control; ALLOCDBG + sandbox.
- result: Z1 and Z2 measure IDENTICALLY (sandbox 23 at 133 insns, out2 3 refs / live 43 = 697, pa4 6/93 = 1290) — cse1 reassociated `(plus (plus a4 32) -32)` to `a4` before flow counted anything. E-s18-2 therefore covers the subtract direction and derivations whose result is consumed function-wide.
- verdict: KILLED

## [s20] The do-while(0) wrap family can replace candidate.c's F1 chain-extender at the same floor on the goto chassis.
- mechanism: Both buy reference counts; if the wraps are insn-count-neutral (s18 T1/T3 measured 132) the seat-correct wrap form should score what candidate.c scores.
- probe: Z3 = Y1 with the ledger's `out3 = out2;` block-2 spelling — i.e. the wrap form that reproduces candidate.c's exact seat table.
- result: ALL-TARGET seats at 132 build insns (out2 5/47 = 2127 tying tbl 5/47, broken the right way by allocno number 79 < 86; pa4 7/95 = 1473) yet sandbox 8 against candidate.c's 1. objdump attributes every extra diff to sched1 emission order under the LOOP notes: `tbl++` (`addiu $s5,$s5,0x4`) displaced past the `addiu $a0,$sp,0x10` / `addu $a1,$s7,$zero` setup, `addu $s0,$s0,$s2` following it, `sw $t0,0x18($sp)` displaced in block 0.
- verdict: KILLED — loop1 wraps are insn-count-neutral but not emission-order-neutral, so on this chassis they are strictly worse than the F1 chain-extender even at identical seats.

## [s20] ENUMERATION — target's block-2 `addiu $s3,$s7,0x20` and target's callee-saved seats are simultaneously unreachable, byte-free, on the goto chassis.
- mechanism: With pa4 at its natural 7 refs / 95 = 1473, the seat order needs out2 in (1473, tbl); the quantised solutions are 4 refs at live 42 (1904, needs the tbl wrap) or 4 refs at live 47 (1702, ties an unwrapped tbl the right way). Both require a FOURTH flow-counted out2 reference, and out2 exists in exactly three regions.
- probe: block 0 = E-s20-1/E-s20-2/E-s20-3 (measured); loop1 = E-s18-3's +2-or-nothing derivation + E-s16-4's measured fold-backs; block 2 = E-s17-3/E-s14-5 plus a read of target's block-2 insns for a possible combine consumer.
- result: All three sites dead. Block 0: weighting co-weights pa4, parameter-sourcing is inert, splitting materialises a copy, cross-derivation reassociates. loop1: a use-only reference cannot survive combine's merge. Block 2: the reference survives cse1 but combine can only delete it by merging into a same-block consumer, and target's block 2 (`addiu $s1/$s2,0x6C`, `addiu $s4,zero,0x12`, `lw $t0,0x18($sp)`, `addiu $s3,$s7,0x20`, `addiu $s0,$t0,0x750`) has none — the un-absorbed copy IS the residual `move $s3,$s6`.
- verdict: CONFIRMED (closure). The live routes are now the real-loop chassis (s19's +1 dial, residual = loop.c's constant-hoist tax) and making the Y2b/Y4 parameter copy free.

## [s20] A do-while(0) wrap in BLOCK 0 can supply out2's missing flow-counted reference on the goto chassis (s19's E-s19-6 concluded block-0 wraps weight nothing).
- mechanism: flow.c:2081 (reg_n_refs += loop_depth) counts every reference in an insn enclosed by LOOP_BEG/END notes twice. s19's W10 wrapped a redundant COPY, which cse1 deletes before flow ever counts it; out2's DEFINITION is a plus, which cse1 cannot delete, so it should be counted at depth 2.
- probe: Form Y1 = s18 V7 with the costly out2-CALL wrap replaced by `do { out2 = (s32 *)(((u8 *)pa4) + 0x20); } while (0);` in block 0; ALLOCDBG via tmp/grind/func_80041188/s20/dump20.sh + sandbox --disable all.
- result: The lift is real and byte-free: out2 3 -> 4 refs at live 42, priority 714 -> 1904 (exactly inside the required (pa4 1473, tbl 2127) window), 132 build insns. But the same wrap co-weights pa4 (7 -> 8 refs, 1473 -> 2526), which jumps pa4 above out2, tbl AND i at once: sandbox 27. out2's only block-0 insn references pa4 by construction, so a block-0 wrap can never lift out2 alone.
- verdict: KILLED

## [s20] Sourcing the wrapped block-0 definition from the PARAMETER a4 instead of the local copy pa4 keeps the wrap's second +1 off pa4.
- mechanism: If a4 and pa4 are distinct pseudos the wrapped insn references a4, leaving pa4 at its natural 7 refs / 1473, below out2's wrapped 1904.
- probe: Y2a (declaration `s32 *pa4 = a4;` kept, wrapped def written from a4) and Y2b (declaration demoted to `s32 *pa4;`, `pa4 = a4;` written as a statement AFTER the a4-sourced wrapped def); ALLOCDBG + sandbox + objdump differ (s20/odiff.py).
- result: Y2a is INERT (ALLOCDBG byte-identical to Y1, pa4 8/95 = 2526, sandbox 27): cse1 canonicalises a4 to pa4 inside block 0's EBB, pa4 being the later-dying register (s17 make_regs_eqv law, cse.c:844-857). Y2b DOES keep them distinct and produces ALL-TARGET seats together with target's block-2 `addiu $s3,$s7,0x20` -- the first time on the goto chassis (out2 4/43=1860 $s6, pa4 6/93=1290 $s7, stptr $s3, i $s4, tbl $s5, stptr2 $s0, a3 $fp) -- but at 133 insns, sandbox 12: the split makes a4 a block-0-LOCAL quantity, so local_alloc seats it in $v0 before global_alloc sees pa4 and `addu $s7,$v0,$zero` is emitted where target has `lw $s7,0x58($sp)` straight into the seat. Y4 (same split, stptr via the F1 chain instead of the stptr wrap) measures identically 12/133; Y9 (tbl wrap also dropped) 15/133.
- verdict: KILLED

## [s20] A semantically real back-derivation `pa4 = (s32 *)((u8 *)out2 - 0x20);` gives out2 a fourth reference with no wrap at all.
- mechanism: E-s18-2 killed forward plus-derivations inside the definition's EBB; reversing the direction puts the surviving reference on out2 rather than on pa4.
- probe: Z1 with Z2 (`pa4 = a4;` written plainly) as the control; ALLOCDBG + sandbox.
- result: Z1 and Z2 measure IDENTICALLY (sandbox 23 at 133 insns, out2 3 refs / live 43 = 697, pa4 6/93 = 1290): cse1 reassociated (plus (plus a4 32) -32) to a4 before flow counted anything. E-s18-2 now provably covers the subtract direction and derivations whose result is consumed function-wide.
- verdict: KILLED

## [s20] The do-while(0) wrap family can replace candidate.c's FAKE F1 chain-extender at the same floor on the goto chassis.
- mechanism: Both buy reference counts, and s18 measured the tbl-load and stptr-store wraps as insn-count-neutral (132), so the seat-correct wrap form should score what candidate.c scores.
- probe: Z3 = Y1 with the ledger's `out3 = out2;` block-2 spelling, i.e. the wrap form that reproduces candidate.c's exact seat table; sandbox + objdump differ.
- result: ALL-TARGET seats at 132 build insns (out2 5/47 = 2127 tying tbl 5/47, broken the right way by allocno number 79 < 86; pa4 7/95 = 1473; stptr 6/41 = 2926) yet sandbox 8 against candidate.c's 1. Every extra diff is sched1 emission order under the LOOP notes: `tbl++` (addiu $s5,$s5,0x4) displaced past the addiu $a0,$sp,0x10 / addu $a1,$s7,$zero setup, addu $s0,$s0,$s2 following it, sw $t0,0x18($sp) displaced in block 0.
- verdict: KILLED

## [s20] ENUMERATION: target's block-2 addiu $s3,$s7,0x20 and target's callee-saved seats are simultaneously unreachable, byte-free, on the goto chassis.
- mechanism: With pa4 at its natural 7 refs / live 95 = 1473 the seat order needs out2 in (1473, tbl); floor_log2 quantisation admits only 4 refs at live 42 (1904, needs the tbl wrap) or 4 refs at live 47 (1702, ties an unwrapped tbl the right way). Both demand a FOURTH flow-counted out2 reference, and out2 exists in exactly three regions of the function.
- probe: block 0 measured this session (Y1 / Y2a / Y2b / Z1); loop1 by E-s18-3's +2-or-nothing derivation plus E-s16-4's measured fold-backs; block 2 by E-s17-3 / E-s14-5 plus a direct read of target's block-2 insns for a possible combine consumer.
- result: All three sites dead. block 0: weighting the definition co-weights pa4, parameter-sourcing is inert, splitting the copy materialises an insn, cross-derivation reassociates. loop1: a use-only extra reference cannot survive combine's merge. block 2: the reference survives cse1 (different EBB) but combine can only delete it by merging into a same-basic-block consumer, and target's block 2 (addiu $s1/$s2,0x6C, addiu $s4,zero,0x12, lw $t0,0x18($sp), addiu $s3,$s7,0x20, addiu $s0,$t0,0x750) contains no insn that could absorb one -- the un-absorbed copy IS the standing residual move $s3,$s6.
- verdict: CONFIRMED

## [s21] E-s20-5's block-0 kill is chassis-relative: DELETING the `pa4` local removes the co-weighting that killed Y1, so a block-0 do-while(0) wrap around out2's definition can lift out2 alone.
- mechanism: flow.c:2081 weights every reference in the enclosed INSN, and out2's definition necessarily references the matrix pointer. On the pa4 chassis that second +1 lands on pa4 at 7 -> 8 refs, and floor_log2(8) = 3 makes the priority JUMP (1473 -> 2526) above out2, tbl and i. With no `pa4` local at all the same insn references the PARAMETER a4, whose live length is 190 (it spans both loops) instead of 95 -- so a4 at 8 refs / 190 is 1263, which crosses the same floor_log2 boundary but lands BELOW out2 and above a3 (808). The wrap's unavoidable second +1 becomes harmless.
- probe: N1 = s20's Y1 with `s32 *pa4 = a4;` deleted and every `pa4` rewritten to `a4` (target's block-2 spelling `out3 = (s32 *)((u8 *)a4 + 0x20);` kept); ALLOCDBG via tmp/grind/func_80041188/s21/dump20.sh + sandbox --disable all + objdump differ.
- result: ALL-TARGET callee-saved seats WITH target's block-2 `addiu $s3,$s7,0x20` at 132 build insns -- stptr 6/41=2926 $s3, stptr2 6/48=2500 $s0, i 8/97=2474 $s4, tbl 5/47=2127 $s5, out2 4/42=1904 $s6, a4 8/190=1263 $s7, a3 4/99=808 $fp, out3 3/47 $s3 -- sandbox 7 where the same construct on the pa4 chassis scored 27. Every remaining diff is sched1 emission ORDER, not allocation.
- verdict: CONFIRMED

## [s21] On the pa4-free chassis tbl needs no wrap at all if its definition is demoted from a declaration initialiser to the LAST statement of block 0.
- mechanism: tbl's live length is measured from its defining insn to `tbl++` in loop1, so moving the definition down block 0 shortens it 47 -> 41 without touching its reference count; at 4 refs the priority rises 1702 -> 1951, which clears out2's wrapped 1860 without the +1 the loop-note dial would have to supply.
- probe: Q1 = N3 (F1 stptr, no loop1 wrap) with `s32 *tbl;` + `tbl = D_80094CFC;` as the final block-0 statement; positional controls Q2/Q3/Q4 (after `stptr = base;` / after the out2 wrap / after `saved`) and P3/P4/P5 (the same positions with the tbl assignment still wrapped).
- result: Q1 scores 7 at 132 insns with ALL-TARGET seats (tbl 4/41=1951 > out2 4/43=1860), i.e. identical to the two-wrap N1/N3 forms with one fewer FAKE construct. The position ladder is monotone: Q1 (last) 7, Q2 8, Q3 8, Q4 (after saved) 11, declaration-initialiser (N6) 11 -- the last two because tbl's live length reaches 45..47 and its priority falls under out2's.
- verdict: CONFIRMED

## [s21] The residual on the pa4-free chassis is a positional DILEMMA on tbl's definition, not a missing reference.
- mechanism: target emits `addiu $s4,zero,1` / `sw $s5,0x34($sp)` / `lui $s5` / `addiu $s5,$s5` as the first four insns of block 0, so target's tbl definition is early in LUID; sched1 cannot schedule a late-LUID block-0 insn with no in-block successors up to the top. But an early tbl definition means live 47, at which 4 references give 1702 -- under out2's 1860/1904 -- so tbl needs a FIFTH reference, and the loop-note dial is the only construct in this compiler that produces an odd reference delta (E-s18-3), so it needs a second wrap, whose notes then cost emission order somewhere else.
- probe: both horns measured. Early tbl + loop1 wrap on `offset = (*tbl) * 6;` (N1/N3); early tbl + block-0 wrap on the tbl assignment at five positions (N4/N5/P3/P4/P5); one wrap enclosing BOTH definitions (T1/T3/T4); the wrap moved onto `tbl++` (N9); and eight block-0 statement permutations on the Q1 chassis (R1..R8).
- result: Nothing beats 7. Early-tbl forms pay ~6 loop1 order diffs (tbl++ hoisted past the `addiu $a0,$sp,0x10` / `addu $a1,$s7,$zero` setup, `addu $s0,$s0,$s2` following) plus 1 block-0 diff; late-tbl forms pay 4 block-0 positions (the `sw $s5,0x34($sp)` / `lui $s5` / `addiu $s5` trio ~7 slots late, `sw $t0,0x18($sp)` 2 slots early). N9 materialises an insn (133). T3 = 8, T1 = 12, T4 = 13, N4 = 13, R1..R8 = 8..14.
- verdict: CONFIRMED

## [s21] A dead store `out3 = out2;` immediately before `out3 = (s32 *)((u8 *)a4 + 0x20);` buys out2 its fourth flow-counted reference for free (the E-s20-5 site-3 escape).
- mechanism: flow.c fixes reg_n_refs before combine, so a reference that a LATER pass deletes is still counted (this is exactly how the F1 chain-extender works); a store killed by an immediately following store to the same pseudo emits nothing, so the form would be byte-free.
- probe: S1 = the pa4-free chassis with the block-0 out2 wrap removed and the dead store inserted; ALLOCDBG + sandbox.
- result: KILLED, and cleanly. 132 build insns (the store really is byte-free) but out2 stays at 3 refs / live 42 = 714 and the seats collapse (a3 takes $s6, a4 $s7, out2 $fp), sandbox 10. flow.c's own dead-code elimination runs INSIDE life analysis and deletes the insn as it scans, so its registers are never counted -- unlike a combine deletion, which happens after counting. The distinction "deleted by flow = uncounted, deleted by combine = counted" is the operative law for every byte-free-reference hunt on this function.
- verdict: KILLED

## [s21] The `pa4` local is chassis-defining, not cosmetic: candidate.c does NOT survive its removal.
- mechanism: on candidate.c's chassis (`out3 = out2;`, no out3-from-a4 definition) the matrix pointer has only 6 references without the `pa4 = a4` copy insn; 6 refs over live 190 is 631, below a3's 808.
- probe: CANDF = candidate.c verbatim with `s32 *pa4 = a4;` deleted and pa4 -> a4 throughout; ALLOCDBG + sandbox.
- result: a4 6/190 = 631 and a3 4/99 = 808 SWAP seats ($s7 <-> $fp); sandbox 1 -> 13 at 132 insns. The pa4-free chassis works only where out3 is defined from a4, because that definition is the 8th reference that pushes a4 across the floor_log2 boundary to 1263.
- verdict: CONFIRMED

## [s21] E-s20-5's block-0 kill is chassis-relative: with the `pa4` local deleted and the parameter `a4` written at every use site, a block-0 do-while(0) wrap around out2's definition lifts out2 without the co-weighting that killed s20's Y1.
- mechanism: flow.c:2081 weights every reference in the ENCLOSED INSN, and out2's definition necessarily references the matrix pointer. On the pa4 chassis that second +1 puts pa4 at 8 refs / live 95, and floor_log2(8)=3 makes the priority jump 1473 -> 2526, above out2, tbl and i. With no pa4 local the reference lands on the parameter a4, whose live length is 190 (it spans both loops), so 8 refs give 1263 -- across the same floor_log2 boundary but landing between out2 (1904) and a3 (808).
- probe: N1 = s20's Y1 with `s32 *pa4 = a4;` deleted and pa4 -> a4 throughout, keeping target's block-2 spelling `out3 = (s32 *)((u8 *)a4 + 0x20);`. ALLOCDBG via tmp/grind/func_80041188/s21/dump20.sh, sandbox --disable all, objdump differ via s21/odiff.py.
- result: ALL-TARGET callee-saved seats WITH target's block-2 addiu $s3,$s7,0x20 at 132 build insns of 132: stptr 6/41=2926 $s3, stptr2 6/48=2500 $s0, i 8/97=2474 $s4, tbl 5/47=2127 $s5, out2 4/42=1904 $s6, a4 8/190=1263 $s7, a3 4/99=808 $fp, out3 3/47 $s3. Sandbox 7, where the identical construct scored 27 on the pa4 chassis. Every remaining diff is sched1 emission order.
- verdict: CONFIRMED

## [s21] On the pa4-free chassis tbl needs no reference lift at all: demoting its declaration initialiser to a plain assignment as the LAST statement of block 0 raises its priority above out2 by shortening its live range.
- mechanism: reg_live_length(tbl) is the distance from tbl's defining insn to `tbl++` in loop1, so moving the definition down block 0 shortens it 47 -> 41 with the reference count unchanged; at 4 refs the priority rises 1702 -> 1951, clearing out2's wrapped 1860 without needing the loop-note dial's +1.
- probe: Q1 (tbl assignment last in block 0, no tbl wrap) plus the full position ladder Q2/Q3/Q4/N6 unwrapped and P3/P4/P5/N4/N5 wrapped; ALLOCDBG + sandbox on each.
- result: Q1 = sandbox 7 at 132 insns with ALL-TARGET seats and ONE fewer FAKE construct than N1 (tbl 4/41=1951 > out2 4/43=1860). Ladder: last 7, after `stptr = base;` 8, after the out2 wrap 8, after `saved` 11, declaration initialiser 11. Wrapping the tbl assignment as well buys nothing anywhere (13/8/8/8/7).
- verdict: CONFIRMED

## [s21] The residual on the pa4-free chassis is a single positional DILEMMA on tbl's definition, not a missing reference.
- mechanism: target's first four block-0 insns are addiu $s4,zero,1 / sw $s5,0x34($sp) / lui $s5 / addiu $s5,$s5, so target's tbl definition is early in LUID, and sched1 cannot lift a late-LUID block-0 insn with no in-block successors to the top. But an early definition means tbl live 47, where 4 refs = 1702 sits under out2's 1860/1904, so tbl then needs a FIFTH reference; per E-s18-3 the loop-note dial is the only construct in GCC 2.7.2 producing an odd reference delta, so a second do-while(0) wrap is required and its notes cost emission order elsewhere.
- probe: Both horns plus permutations: early tbl + loop1 wrap (N1/N3), early tbl + block-0 tbl wrap at five positions (N4/N5/P3/P4/P5), one wrap enclosing both definitions (T1/T3/T4), the wrap moved onto `tbl++` (N9), and eight block-0 statement permutations of Q1 (R1..R8).
- result: Nothing beats 7. Early-tbl forms pay ~6 loop1 order diffs (tbl++ hoisted past the addiu $a0,$sp,0x10 / addu $a1,$s7,$zero setup, addu $s0,$s0,$s2 dragged with it) plus one block-0 diff; late-tbl forms pay 4 block-0 positions (the sw $s5 / lui $s5 / addiu $s5 trio ~7 slots late, sw $t0,0x18($sp) 2 slots early). N9 materialises an insn (133, sandbox 21); T3 8, T1 12, T4 13, N4 13, R1..R8 8..14.
- verdict: CONFIRMED

## [s21] A dead store `out3 = out2;` immediately before `out3 = (s32 *)((u8 *)a4 + 0x20);` buys out2 its fourth flow-counted reference for free -- the byte-free escape from E-s20-5's site 3.
- mechanism: flow.c fixes reg_n_refs before combine runs, so a reference deleted by a LATER pass is still counted (this is how the F1 chain-extender works), and a store killed by an immediately following store to the same pseudo emits nothing, so the form should be byte-free AND counted.
- probe: S1 = the pa4-free chassis with the block-0 out2 wrap removed and the dead store inserted; ALLOCDBG + sandbox.
- result: The store IS byte-free (132 build insns) but flow-INVISIBLE: out2 stays at 3 refs / live 42 = 714 and the seats collapse (a3 -> $s6, a4 -> $s7, out2 -> $fp), sandbox 10. flow.c's dead-code elimination runs INSIDE life analysis -- propagate_block deletes the insn during the same backward scan that increments reg_n_refs -- so its registers are never counted. Deleted-by-flow is uncounted; only deleted-by-combine is counted.
- verdict: KILLED

## [s21] The `pa4` local in candidate.c is a redundant copy that can be deleted without changing candidate.c's codegen.
- mechanism: cse1 canonicalises a4 and pa4 into one quantity (s17 make_regs_eqv law), so the copy looked cosmetic.
- probe: CANDF = candidate.c verbatim with `s32 *pa4 = a4;` deleted and pa4 -> a4 throughout; ALLOCDBG + sandbox.
- result: KILLED. Without the copy insn the matrix pointer has only 6 references, and 6/190 = 631 falls BELOW a3's 4/99 = 808, so a4 and a3 swap $s7/$fp: sandbox 1 -> 13 at 132 insns. The pa4-free chassis works only where `out3 = (s32 *)((u8 *)a4 + 0x20);` supplies an 8th reference. The two chassis are disjoint.
- verdict: KILLED

## s22 (synthesis, 2026-08-25) — verdicts on the s21 frontier, and the frontier RESET

Chassis: candidate.c = sandbox 1 (unchanged floor). Controls re-measured this session:
W4 (real-loop chassis) = 3 at 132/132; Q1/A0 (pa4-free goto chassis) = 7 at 132/132.
Evidence: `evidence.md` E-s22-1 .. E-s22-5. Artifacts: `tmp/grind/func_80041188/s22/`.

### s21-F3 (the real-loop chassis, pa4-free, may be strictly better than both goto horns) — KILLED
- probe: applied W4, dumped `-da` including the loop dump, and read the movable decision
  numerically; then derived `threshold` from mips.h's FIXED_REGISTERS; then measured the
  one eligibility test that C can falsify (W4b, the stored constant made a user variable
  inside the loop body).
- result: `Loop from 42 to 172: 48 real insns.` / `Insn 155: regno 122 (life 1),
  move-insn savings 1  moved to 315`; threshold = 1 * (1 + 60) = 61 >= insn_count = 48
  with a margin of 13 real insns; the three scan_loop tests are an OR and each passes
  independently, W4b measuring 3 at 132/132 (byte-identical to W4). The hoisted pseudo
  is live across every call, so it must spill and reload rematerialises `li 2` into $t0
  against target's $v0.
- verdict: KILLED — and not just for this frontier item: EVERY form of this function
  with loop1 as a real C loop is permanently 2 diffs behind, so the whole real-loop
  chassis (s19 W4/W6/W8, s18 V5's regime) is FORECLOSED for a byte-exact match. The
  loop-note reference dial stays available only as an isolated `do { } while (0)` wrap.

### s21-F2 (a dependency, not a position, could fix tbl's late definition) — KILLED
- probe: enumerated every insn in target's block 0 (asm/funcs/func_80041188.s:2-28) and
  asked which of them could legitimately be a function of `D_80094CFC`, i.e. could serve
  as a zero-insn in-block consumer giving tbl's `lui`/`addiu` pair an INSN_PRIORITY.
- result: none. Every block-0 value derives from a parameter, from the `D_800A9A10[a0]`
  load, or from a literal. There is no consumer to give tbl, so sched.c's
  `rank_for_schedule` will always sink a late-LUID tbl definition.
- verdict: KILLED by derivation. The late-tbl horn's 4-position emission-order cost is
  structural; the s21 dilemma has only its early-tbl horn left, which needs a fifth tbl
  reference (odd delta ⇒ loop-note only) whose notes then cost emission order elsewhere.

### s21-F1 (early tbl + out2 pushed below 1702) — half KILLED, half re-stated
- probe: form A1 = Q1 with the block-0 FAKE wrap replaced by an honest split-init chain
  on out2 sourced from the parameter (`out2 = (s32 *) a4; out2 = ... + 0x20;`), plus A2
  (the same with tbl restored to a declaration initialiser). Read `red.i.cse` and
  `red.i.flow` for the fate of the chain.
- result: both 10 at 132/132; the chain is byte-free but yields NO reference. cse1
  rewrites the second insn's source to `(plus (reg 76) 32)` because the parameter
  outlives out2 and is therefore the canonical register of the equivalence class
  (make_regs_eqv, cse.c:844-857); the first insn goes dead and flow.c deletes it inside
  life analysis, so it is never counted.
- verdict: the "F1 chain-extender supplies out2's fourth reference" half is KILLED for
  every chassis by the derived iff-law (a chain-extender pays IFF the recipient outlives
  the donor, and the matrix pointer always outlives out2). The "out2 pushed below 1702"
  half is not so much killed as re-stated by A1's table: with everything natural, out2
  sits at 714, four seats are already target's, and the defect is a3 (808) above out2 —
  every quantised repair still routes through out2 at 4 references.

## s22 frontier (RESET — the strongest three for the next ladder pass)

The 22-session search has converged: three chassis are down to one live chassis and one
live atom. State the atom precisely, because every remaining probe is a test of it.

**THE ATOM.** out2 needs a FOURTH flow-counted reference that is byte-free. A reference
is flow-counted iff its insn is still present when flow.c's life analysis runs, and it
is byte-free iff the insn is gone from the final output. flow.c deletes dead insns
INSIDE life analysis (E-s21-5), so "byte-free by being dead" is self-defeating. combine
runs AFTER flow's counting, so the ONLY viable shape is: **an insn that is live and
useful through cse1 and flow, and that combine then merges into a consumer.** All three
of the known deliveries fail a clause: `out3 = out2` is live but never merged (it IS the
residual insn); dead stores and copy chains are deleted by flow; the do-while(0) wrap
does not add a reference at all, it re-weights an existing one (and is FAKE + costs
emission order).

1. **The combine-merge enumeration: which 2-insn RTL pattern containing an out2
   reference does GCC 2.7.2's `try_combine` fold into an insn target already emits?**
   - mechanism: combine.c's 2->1 and 3->2 substitution runs after flow has counted
     reg_n_refs, so a reference in an insn combine later absorbs is counted and free —
     this is exactly the shape E-s16-5 proved target's own `stptr` reference has. The
     candidate consumers are target's own block-2 insns (`addiu $s1/$s2,0x6C`,
     `addiu $s4,zero,0x12`, `lw $t0,0x18($sp)`, `addiu $s3,$s7,0x20`,
     `addiu $s0,$t0,0x750`) and its block-0 tail.
   - next probe: work from combine.c's side rather than from C. Enumerate the
     `try_combine` patterns whose OUTPUT is `(set reg (plus reg const))` or
     `(set reg (mem ...))` and whose deleted INPUT insn mentions a third register; for
     each, write the C that generates that input pair with out2 as the third register
     and measure insn count FIRST (a surviving insn is fatal, as in E-s20-2). Note in
     advance that `out3 = (s32 *)((u8 *)out2 + 0x20 - 0x20)` and friends are already
     dead by E-s20-3 (cse1 reassociates), so the pair must not be arithmetically
     collapsible before combine sees it.

2. **Push a3 below out2 instead of lifting out2** — the mirror image nobody has probed,
   and A1's table makes it a two-number question for the first time.
   - mechanism: A1 has a3 4 refs / live 99 = 808 immediately above out2's 3/42 = 714,
     and a4 7/190 = 736 between them. a3's priority is `floor_log2(4)*4/live*10000`, so
     a3 falls below 714 at live >= 114 and below 736 at live >= 109 — a live-length
     question, not a reference question, and live length is the dial s21 showed is
     movable by declaration position and by which loops a value spans. a3 is the plain
     `s32 a3` parameter passed to three calls.
   - next probe: sweep a3's live length upward on the A1 chassis — the parameter homing
     copy's position, and whether a3 can be made live past its last call (it is target's
     `$fp`, the LAST allocated seat, so a longer live range costs nothing structurally).
     Read a3/a4/out2 priorities out of ALLOCDBG before looking at the score; the target
     order needs out2 > a4 > a3 and A1 currently has a3 > a4 > out2, so a3 must fall by
     two places, which may need a4 lifted at the same time (a4 reaches 1263 at 8 refs,
     which is what the block-0 wrap buys today).

3. **Settle the family status of candidate.c's `stptr = base; stptr += 0xFC;` NOW, not
   at submission time** — it is the difference between a floor-1 candidate that is one
   insn from done and a floor-1 candidate that cannot be landed at all.
   - mechanism: E-s22-2 shows this construct is not an arbitrary reference pump. It pays
     its +1 reference *because* `base` dies at the split point, so out2-style chains
     provably cannot be spelled the same way; combine then merges the pair back to
     target's single `addiu $s3,$v0,0xFC`. That is a structural property with a named
     pass on both ends (cse.c:844-857 make_regs_eqv for survival, combine.c for the
     merge). Separately, `feedback/split-init-accumulation-sanctioned.md` records the
     owner sanctioning same-variable split-init accumulation (`var = a; var += b`) as a
     pure-C technique on 2026-06-13, while the standing owner directive for this
     function says the chain-extender "still needs FAKE annotation or replacement on
     land", and the frozen family list carries the F1 chain-extender as a FAKE-annotated
     last resort.
   - next probe: this is a `ruling-request`, not a measurement — ask whether
     `stptr = base; stptr += 0xFC;` is (i) the sanctioned split-init-accumulation family,
     (ii) the F1 chain-extender family needing only the `/* FAKE: ... */` annotation
     (which is a one-comment fix, per the standing note that an annotation-FORMAT FAIL is
     not a wall), or (iii) neither. Do not spend a session's measurements on the
     one-insn residual until the answer is known, because (iii) would move the target.

## [s22] s21 frontier item 3 — the real-loop chassis (s19 W4), rebuilt pa4-free, may be strictly better than both goto horns, because out2's +1 comes free from the loop-note dial there.
- mechanism: loop.c emits LOOP_BEG/END notes only for C loop constructs; flow.c:2081 then weights every reference inside the loop. s19 E-s19-3 had asserted the counterweight (loop.c's hoist of the invariant `li 2`) 'passes for ANY lifetime', without numbers.
- probe: Applied W4 to src/text1a_pre.c (sandbox 3 at 132/132, table reproduced allocno-for-allocno), dumped with -da and READ the loop dump's movable decision; derived threshold from mips.h FIXED_REGISTERS; then measured the one eligibility clause a C spelling can falsify (W4b: the stored constant made a user variable inside the loop body).
- result: tmp/grind/func_80041188/s22/W4/red.i.loop:109-111 prints 'Loop from 42 to 172: 48 real insns.' and 'Insn 155: regno 122 (life 1), move-insn savings 1  moved to 315'. The loop.c:1631 test is threshold*savings*lifetime >= insn_count = 61*1*1 >= 48, where threshold = (loop_has_call?1:2)*(1+n_non_fixed_regs) = 1*(1+60); n_non_fixed_regs = 60 because mips.h:1188 FIXED_REGISTERS holds 8 ones out of FIRST_PSEUDO_REGISTER = 68 (mips.h:1181). The margin is 13 real insns and insn_count is the only C-side term. The three scan_loop tests at loop.c:686-700 are an OR whose clauses each pass independently, so W4b measures sandbox 3 at 132/132, byte-identical to W4. The hoisted pseudo (3 refs / live 92) is live across every call, so it cannot take a call-clobbered reg and a callee-saved one would cost a save/restore pair; global.c spills it and reload rematerialises `li 2` into $t0 where target's local-alloc-owned in-place li gets $v0 (s22/W4/red.s:258-259).
- verdict: KILLED

## [s22] s21 frontier item 1 (honest half) — out2's fourth flow-counted reference can be bought byte-free by an F1-style split-init chain-extender sourced from the matrix pointer, replacing the block-0 do-while(0) FAKE wrap on the pa4-free chassis.
- mechanism: The same construct that pays stptr its 7th reference in candidate.c (`stptr = base; stptr += 0xFC;`): two insns before combine, one after, so the extra reference is counted by flow and then merged away.
- probe: Form A1 = s21's Q1 with the wrap replaced by `out2 = (s32 *) a4; out2 = (s32 *)(((u8 *) out2) + 0x20);` (and A2 = the same with tbl restored to a declaration initialiser). Measured sandbox and build_insns, then read tmp/grind/func_80041188/s22/A1/red.i.cse and red.i.flow to trace the chain's fate.
- result: A1 = sandbox 10 at 132 of 132 build insns (byte-free, unlike the pa4-chassis attempt which materialised a copy), A2 = 10 — but out2 stays at 3 refs / live 42 = 714 and is seated $fp. red.i.cse shows insn 37's source rewritten from (plus (reg 85) 32) to (plus (reg 76) 32): reg 76 is the parameter a4, which OUTLIVES out2, so by make_regs_eqv (cse.c:844-857) a4 is the equivalence class's canonical register. Insn 34 (`out2 = a4`) therefore goes dead and is ABSENT from red.i.flow — flow.c deletes dead insns inside life analysis, so its registers are never counted (E-s21-5).
- verdict: KILLED

## [s22] s21 frontier item 2 — tbl's late definition can be lifted to target's early emission position by giving its lui/addiu pair an in-block successor, so sched.c ranks it on INSN_PRIORITY instead of sinking it on INSN_LUID.
- mechanism: sched.c's rank_for_schedule ranks by the dependency critical path and falls back to LUID; the requirement is a block-0 insn target ALREADY emits that could legitimately be sourced from tbl, so the dependency costs zero insns.
- probe: Enumerated every insn of target's block 0 (asm/funcs/func_80041188.s:2-28) and tested each for being a function of D_80094CFC.
- result: None qualifies. Block 0 is exhaustively: the frame addiu; nine callee-saved sw's plus sw $ra; the parameter homing copies addu $s1,$a1 / addu $s2,$a2 / addu $fp,$a3; addiu $s4,zero,1 (i); tbl's own lui/addiu; sll $a0,$a0,2 + lui $at / addu $at,$a0 / lw $v0,%lo(D_800A9A10)($at) (base); lw $s7,0x58($sp) (a4); addiu $t0,$v0,0x94 (ents); addiu $s6,$s7,0x20 (out2); addiu $s3,$v0,0xFC (stptr); sw $t0,0x18($sp). Every value derives from a parameter, from the D_800A9A10[a0] load, or from a literal.
- verdict: KILLED

## [s22] With every construct removed, the pa4-free goto chassis is far from target's allocation and the residual is diffuse.
- mechanism: 22 sessions of ledger framing treated the seat vector as needing several simultaneous lifts (out2, tbl, pa4/a4), which is why every form carries two or more constructs.
- probe: Read A1's ALLOCDBG table — A1 is the first measured form with NO wrap, NO tbl position trick and out2 at its natural 3 references.
- result: stptr 7/41=3414 $s3, stptr2 6/48=2500 $s0, i 8/97=2474 $s4, tbl 4/47=1702 $s5, a3 4/99=808 $s6, a4 7/190=736 $s7, out2 3/42=714 $fp, out3 3/47 $s3. FOUR of the contested callee-saved seats are already target's with zero constructs, and tbl already sits at exactly the early-definition priority (1702) that s21's dilemma wanted. The single defect is that a3 (808) outranks out2 (714) and a4 (736).
- verdict: CONFIRMED

## s23 (synthesis, 2026-08-25) — verdicts on the s22 frontier, and the frontier RESET

Chassis: HEAD = 27; candidate.c = **sandbox 1 at 132/132** (floor unchanged, re-measured at
session start and again at the end). Evidence: `evidence.md` E-s23-1 .. E-s23-5. Artifacts:
`tmp/grind/func_80041188/s23/`. src/text1a_pre.c restored to HEAD; no build-pipeline file
touched.

## [s23] s22-F2 — a3 can be pushed BELOW a4 and out2 by lengthening its live range, inverting the seat order from the bottom instead of the top.
- mechanism: global.c's priority is floor_log2(refs)*refs/live*10000, so at a3's structural 4 references its priority is purely a live-length question: 80000/L < 736 needs L >= 109, 80000/L < 714 needs L >= 114, against a measured 99. s21 had shown live length is movable by declaration position and by which loops a value spans, and a3 is target's $fp (the last-allocated seat), so a longer range costs nothing structurally.
- probe: (i) the whole block-0 out2 position sweep read for a3 (B0, C0, C3, C4); (ii) D1 = loop2's `*((s16 *)(stptr2 + 6)) = 1;` moved BEFORE the func_800523E0 that is a3's last use; (iii) D2 = `i++` moved before that same call; (iv) F1/F2 = an explicit `s32 pa3 = a3;` homing-copy local used at all three call sites, on the candidate chassis and the pa4-free chassis respectively — the exact analogue of the `pa4` local that halves a4's live length 190 -> 95 and lifts it 736 -> 1263. ALLOCDBG on all of them plus sandbox.
- result: a3 measures 4 refs / live 99 / pri 808 in TEN forms without a single exception (B0, C0, C3, C4, D1, D2, D5, D6, F1, F2). D1 = sandbox 15, D2 = 13, F1 = 5, F2 = 15, all at 132/132; the only effect of the pa3 copy anywhere is a1/a2 live 99 -> 100 and the schedule shift that follows. The dial the hypothesis needed to move by +10..+15 does not move by 1.
- verdict: KILLED

## [s23] s22-F1 (site half) — out2's fourth flow-counted reference can be delivered by an insn in BLOCK 2 that survives cse1 and flow and is then absorbed by combine.
- mechanism: combine.c's try_combine runs after flow.c has fixed reg_n_refs, so a reference living in an insn combine later absorbs is both counted and byte-free — the shape E-s16-5 proved target's own stptr reference has. s22 listed target's block-2 insns as the candidate consumers.
- probe: instead of enumerating C spellings, measure which BLOCK the reference may live in at all. D5/D6 = out2's definition moved inside loop1 (the only way to reach the short-live window) on the pa4 chassis with target's block-2 spelling; G1 = out2 defined in block 0 as usual AND recomputed identically at the end of loop1's body. Read red.i.cse / red.i.flow for the fate of block 2's out3 definition in each, against B0/C0/C3/C4 as the block-0-only negative control.
- result: cse.c's `cse_end_of_basic_block` extends an EBB through single-predecessor successors. loop1's head has TWO predecessors, so the EBB starting at block 0 terminates there and nothing computed in block 0 is in cse's table at block 2; block 2 has ONE predecessor, so loop1's EBB extends through it and everything computed inside loop1 IS. Measured: D5/D6 rewrite `out3 = (u8*)pa4 + 0x20` to `out3 = out2` (s23/D5/red.i.flow insn 167, sandbox 23 at 132/132, out2 4/21 = 3809); G1 does the same (s23/G1/red.i.cse insn 170, sandbox 12 at 133/132, out2 5/42 = 2380); B0/C0/C3/C4 do NOT fold and keep target's addiu with out2 at 3 references.
- verdict: KILLED for block 2, and newly KILLED for in-loop1 DEFINITIONS — but the law is a definition law, not a reference law (a USE records no expression in cse's table), so the in-loop1-USE quadrant survives and is where the combine-merge enumeration must now be aimed.

## [s23] NEW — the seat requirement has a second solution at THREE references: on the pa4 chassis out2 seats correctly at live length 18..23 with no fourth reference at all.
- mechanism: on the pa4 chassis a4 = 6/95 = 1263 and tbl = 4/47 = 1702, so out2's required priority band is (1263, 1702); at 3 references that is 30000/L in the band, i.e. L in [18,23]. Ten sessions of ledger framing had treated "out2 needs a 4th reference" as the only route, because only the 4-reference band ([47,63]) had ever been computed.
- probe: D5 (out2 defined inside loop1 just before its first use) and D6 (at the top of loop1's body), both with target's block-2 spelling, ALLOCDBG + sandbox + insn count.
- result: the window IS reachable — out2 measures live 21 in both, at 132 of 132 build insns, i.e. at zero insn cost. But it is unusable twice over: the in-loop1 definition triggers the cse-EBB fold above (out2 lands at 4/21 = 3809, outranking tbl), and independently it emits out2's addiu inside loop1 where target emits `addiu $s6,$s7,0x20` in block 0. Sandbox 23.
- verdict: KILLED (found and closed in the same session)

## [s23] NEW — on the pa4-free chassis, the block-0 POSITION of out2's definition can seat out2 above a4, because a later definition shortens out2's live range.
- mechanism: out2 3 refs -> 30000/L(out2); a4 7 refs -> 140000/L(a4); so out2 outranks a4 iff L(a4) > 4.667 * L(out2). C3/C4 shorten out2 to 41 and the gap to a4 is only 13 priority points, so one more unit of live length would flip it.
- probe: the complete block-0 position sweep of out2's definition — C0 (first statement), B0 (natural), C3 (last statement), C4 (after `tbl = D_80094CFC;`) — ALLOCDBG + sandbox on each.
- result: the position dial moves BOTH live lengths in the same direction, so the ratio barely responds: C0 192/43 = 4.465 (out2 697, a4 729, sandbox 13), B0 190/42 = 4.524 (714 / 736, sandbox 10), C3 and C4 188/41 = 4.585 (731 / 744, sandbox 13). Monotone and exhausted below the required 4.667.
- verdict: KILLED

## [s23] NEW — a same-value recompute of out2 at the end of loop1 (dead-store family, a position no session had tried) buys out2 a flow-counted reference byte-free.
- mechanism: E-s21-5 showed flow-deleted insns are uncounted, but a same-value re-store that CSE recognises as redundant would be deleted by cse1's own redundancy logic AFTER... in fact it would have to survive to flow and then be removed by a later pass; loop1 was untried as a position.
- probe: G1 = candidate chassis with target's block-2 spelling restored and `out2 = (s32 *)(((u8 *) pa4) + 0x20);` re-executed as the last statement of loop1's body. ALLOCDBG, insn count, red.i.cse.
- result: byte-COSTLY — 133 of 132 insns. out2's block-0 value is not in cse1's table for loop1's EBB (E-s23-1), so the recompute is not recognised as redundant and survives to the output. It also fails the other clause: being a definition, it triggers the cse fold and block 2's addiu becomes `out3 = out2` again. out2 5/42 = 2380, above tbl (1666). Sandbox 12.
- verdict: KILLED

## s23 frontier (RESET — the strongest three for the next ladder pass)

**THE ATOM, restated with s23's partition.** out2 needs a FOURTH flow-counted reference
that is (a) present when flow.c counts (so not deleted by flow — E-s21-5), (b) absorbed by
combine (so byte-free), and (c) **sited in BLOCK 0 or as a USE inside loop1** — because a
definition inside loop1, or any recomputation in block 2, is folded by cse1 into the
residual `move $s3,$s6` (E-s23-1). Block 0's deliveries are enumerated and only the FAKE
`do { } while (0)` wrap survives there. That leaves ONE unprobed quadrant.

1. **The combine-merge enumeration, re-aimed at LOOP1's insns: which 2-insn RTL pattern
   containing a USE of out2 does GCC 2.7.2's `try_combine` fold into an insn target already
   emits inside loop1?**
   - mechanism: combine runs after flow has fixed reg_n_refs, so a reference in an insn
     combine absorbs is counted and free (the shape E-s16-5 proved target's own stptr
     reference has). E-s23-1 additionally proves a USE is safe where a DEFINITION is not:
     a use records no expression in cse's table, so block 2's `addiu $s3,$s7,0x20` survives.
     The candidate consumers are target's own loop1 insns — the `addiu $a0,$sp,0x10` buf
     address, the `addu $a1,...` argument moves, the `addiu $a2,$s3,0x38` and `addiu $a3,...`
     argument computations for `func_800523E0`, the `sh` into `6($s3)`, and the
     `addiu $s3,$s3,0x68` walk.
   - next probe: work from combine.c's side. Enumerate the `try_combine` 2->1 patterns whose
     OUTPUT is `(set reg (plus reg const))`, `(set reg (mem ...))` or `(set (mem ...) reg)`
     and whose deleted INPUT insn mentions a third register; for each, write the loop1 C that
     generates that input pair with out2 as the third register, and check build_insns FIRST
     (a surviving insn is fatal — E-s20-2, and now G1). Rule out in advance anything
     arithmetically collapsible before combine sees it (cse1 reassociates — E-s20-3), and
     anything that is a DEFINITION of a value equal to `a4 + 0x20` (E-s23-1).

2. **out2 must have FOUR references — now proven from BOTH sides — so the live axis is the
   ONE all-target-seats form that already exists: s21's Q1 (pa4-free, block-0 do-while(0)
   wrap, target's block-2 spelling, sandbox 7 at 132/132), whose entire residual is seven
   sched1 EMISSION-ORDER diffs. Point `tools/sched_solver` at it — it has never been run on
   this form.**
   - mechanism: the closure argument. a3 is immovable at 808 (E-s23-2), so out2 must exceed
     808; at 3 references that needs live length <= 37 (30000/37 = 810), and the block-0
     position sweep floors out2's live length at 41 (E-s23-3) while the only way below 41 is
     an in-loop1 definition, which is separately dead (E-s23-4). Symmetrically out2 must
     exceed a4, which the ratio law closes at 3 references on the pa4-free chassis and which
     needs 1263 on the pa4 chassis. So every surviving chassis requires out2 at >= 4
     references, and exactly one measured form delivers that with target's block-2 spelling
     at 132 insns: Q1. Its cost is not allocation at all — it is sched.c's ordering of block
     0 (the `sw $s5` / `lui $s5` / `addiu $s5` trio ~7 slots late, `sw $t0,0x18($sp)` 2 slots
     early — E-s21-4), which is exactly what `tools/sched_solver` models order- and
     clock-exactly.
   - next probe: apply Q1 (`memory/grind/func_80041188/alt_Q1_pa4free_alltarget_s21.c`),
     dump `red.i.sched`, and run `tools/sched_solver` on block 0 against target's block-0
     order to get a typed REACHABLE / FORECLOSED verdict plus a ranked C-lever vector for the
     seven diffs. If FORECLOSED, the do-while(0) route is dead as a byte-exact form and the
     function reduces to frontier item 1 alone; if REACHABLE, the levers it names are the
     first concrete instructions for closing a form that already holds every target seat.
     (Note the layer-1 constraint: the BANNED do-while(0) is the loop1-half wrap entered by
     an external goto, NOT a self-contained block-0 wrap around a single statement — but the
     wrap is still a FAKE construct needing its annotation and its family citation.)

3. **Settle the family status of candidate.c's `stptr = base; stptr += 0xFC;` (carried
   unchanged from the s22 frontier — it is still unanswered and still gates landing).**
   - mechanism: E-s22-2 shows the construct is not an arbitrary reference pump: it pays its
     +1 precisely because `base` dies at the split point, so `make_regs_eqv` (cse.c:844-857)
     keeps stptr canonical, the chain survives cse1, and combine merges the pair back into
     target's single `addiu $s3,$v0,0xFC`. Separately,
     `feedback/split-init-accumulation-sanctioned.md` records the owner sanctioning
     same-variable split-init accumulation (`var = a; var += b`) as pure C on 2026-06-13,
     while the standing owner directive for this function calls the same construct an F1
     chain-extender needing FAKE annotation or replacement, and the frozen family list
     carries F1 as a FAKE-annotated last resort.
   - next probe: a `ruling-request`, not a measurement — is `stptr = base; stptr += 0xFC;`
     (i) sanctioned split-init accumulation, (ii) the F1 family needing only the
     `/* FAKE: what + mechanism + lever-exhaustion */` comment, or (iii) neither? Answer
     (iii) moves the target and would invalidate candidate.c as the floor form.

## [s23] s22 frontier item 2 - a3 can be pushed BELOW a4 and out2 by lengthening its live range, inverting the seat order from the bottom instead of lifting out2 from the top.
- mechanism: global.c's allocno priority is floor_log2(refs)*refs/live*10000, so at a3's structural 4 references its priority is purely a live-length question: 80000/L < 736 needs L >= 109 and 80000/L < 714 needs L >= 114, against a measured 99. s21 had shown live length is movable by declaration position and by which loops a value spans, and a3 is target's $fp (the last-allocated seat), so a longer range costs nothing structurally.
- probe: Four independent attacks, all with ALLOCDBG tables read before the score: (i) a3 read out of the whole block-0 out2 position sweep (B0/C0/C3/C4); (ii) D1 = loop2's `*((s16 *)(stptr2 + 6)) = 1;` moved BEFORE the func_800523E0 that is a3's last use; (iii) D2 = `i++` moved before that same call; (iv) F1/F2 = an explicit `s32 pa3 = a3;` homing-copy local used at all three call sites, on the candidate chassis and the pa4-free chassis - the exact analogue of the `pa4` local that halves a4's live length 190 -> 95 and lifts it 736 -> 1263.
- result: a3 measures 4 refs / live 99 / pri 808 in TEN forms without a single exception (B0, C0, C3, C4, D1, D2, D5, D6, F1, F2). D1 = sandbox 15, D2 = 13, F1 = 5, F2 = 15, all at 132 of 132 insns. The only visible effect of the pa3 copy anywhere is a1/a2 live 99 -> 100 and the schedule shift that follows. The dial the hypothesis needed to move by +10..+15 does not move by 1.
- verdict: KILLED

## [s23] s22 frontier item 1 (site half) - out2's fourth flow-counted reference can be delivered by an insn in BLOCK 2 that survives cse1 and flow and is then absorbed by combine.
- mechanism: combine.c's try_combine runs after flow.c has fixed reg_n_refs, so a reference living in an insn combine later absorbs is both counted and byte-free - the shape E-s16-5 proved target's own stptr reference has. s22 listed target's block-2 insns as the candidate consumers.
- probe: Rather than enumerate C spellings, measure which BLOCK the reference may live in at all. D5/D6 = out2's definition moved inside loop1 (the only route to the short-live window) on the pa4 chassis with target's block-2 spelling; G1 = out2 defined in block 0 as usual AND recomputed identically at the end of loop1's body. Read red.i.cse / red.i.flow for the fate of block 2's out3 definition in each, against B0/C0/C3/C4 as the block-0-only negative control.
- result: cse.c's cse_end_of_basic_block extends an EBB through single-predecessor successors. loop1's head has TWO predecessors, so the EBB starting at block 0 terminates there and nothing computed in block 0 is in cse's table at block 2; block 2 has ONE predecessor, so loop1's EBB extends through it and everything computed inside loop1 IS. Measured: D5/D6 rewrite `out3 = (u8*)pa4 + 0x20` into `out3 = out2` (s23/D5/red.i.flow insn 167; sandbox 23 at 132/132; out2 4 refs / live 21 = 3809, the 4th reference IS the fold). G1 does the same (s23/G1/red.i.cse insn 170; sandbox 12 at 133 of 132 insns; out2 5/42 = 2380). B0/C0/C3/C4 do NOT fold and keep target's addiu $s3,$s7,0x20 with out2 at 3 references.
- verdict: KILLED

## [s23] NEW - the seat requirement has a second solution at THREE references: on the pa4 chassis out2 seats correctly at live length 18..23 with no fourth reference at all.
- mechanism: On the pa4 chassis a4 = 6/95 = 1263 and tbl = 4/47 = 1702, so out2's required band is (1263, 1702); at 3 references that is 30000/L inside the band, i.e. L in [18,23]. Ten sessions had treated 'out2 needs a 4th reference' as the only route because only the 4-reference band ([47,63]) had ever been computed.
- probe: D5 (out2 defined inside loop1 just before its first use) and D6 (at the top of loop1's body), both keeping target's block-2 spelling; ALLOCDBG + sandbox + build insn count.
- result: The window IS physically reachable - out2 measures live 21 in both, at 132 of 132 build insns, i.e. at zero insn cost. It is unusable twice over: (i) the in-loop1 definition triggers the cse-EBB fold, so out2 lands at 4/21 = 3809 and outranks tbl; (ii) independently, an in-loop1 definition emits out2's addiu inside loop1 while target emits `addiu $s6,$s7,0x20` in block 0. Sandbox 23.
- verdict: KILLED

## [s23] NEW - on the pa4-free chassis the block-0 POSITION of out2's definition can seat out2 above a4, because a later definition shortens out2's live range.
- mechanism: out2 at 3 references gives 30000/L(out2); a4 at 7 references gives 140000/L(a4); so out2 outranks a4 iff L(a4) > 4.667 * L(out2). C3/C4 shorten out2 to live 41 and the residual gap to a4 is only 13 priority points, so one more unit of live length would flip it.
- probe: The complete block-0 position sweep of out2's definition - C0 (first statement), B0 (natural, after `saved`), C3 (last statement of block 0), C4 (after `tbl = D_80094CFC;`) - ALLOCDBG + sandbox on each.
- result: The position dial moves BOTH live lengths in the same direction, so the ratio barely responds: C0 192/43 = 4.465 (out2 697, a4 729, sandbox 13); B0 190/42 = 4.524 (714 / 736, sandbox 10); C3 and C4 188/41 = 4.585 (731 / 744, sandbox 13). Monotone and exhausted below the required 4.667.
- verdict: KILLED

## [s23] NEW - a same-value recompute of out2 at the end of loop1 (the dead-store family, in a position no session had tried) buys out2 a flow-counted reference byte-free.
- mechanism: E-s21-5 showed flow-deleted insns are uncounted, so the reference must survive flow and be removed later; a same-value re-store of a local was the untried delivery, and loop1 was the untried position.
- probe: G1 = candidate chassis with target's block-2 spelling restored and `out2 = (s32 *)(((u8 *) pa4) + 0x20);` re-executed as the last statement of loop1's body; ALLOCDBG, build insn count, red.i.cse.
- result: Byte-COSTLY: 133 of 132 insns. out2's block-0 value is not in cse1's table for loop1's EBB (the same law), so the recompute is not recognised as redundant and survives to the output. It also fails the other clause - being a definition it triggers the cse fold, so block 2's addiu becomes `out3 = out2` again. out2 5/42 = 2380, above tbl (1666). Sandbox 12.
- verdict: KILLED

## [s23] NEW (closure) - out2 must carry at least FOUR flow-counted references on every surviving chassis.
- mechanism: Derived from the two kills above rather than measured directly. a3 is immovable at 808 (ten forms), so out2 must exceed 808; at 3 references that needs live <= 37 (30000/37 = 810). Symmetrically out2 must exceed a4, which the ratio law closes at 3 references on the pa4-free chassis (max 4.585 vs required 4.667) and which needs 1263 on the pa4 chassis (live <= 23).
- probe: Combine the measured floors: block-0 definitions floor out2's live length at 41 (C3/C4, the whole monotone position sweep), and the only route below 41 is an in-loop1 definition, killed independently by the cse-EBB fold and by the addiu's emission block.
- result: No 3-reference out2 can clear either a3 or a4 on any measured chassis. candidate.c's `out3 = out2;` is therefore not one arbitrary choice among many - it is the only measured delivery of a requirement now proven necessary, and the whole remaining problem is whether a byte-free spelling of that same reference exists.
- verdict: CONFIRMED

# ============================ s24 (synthesis) ============================

## [s24] The residual is a ONE-reference problem: a byte-free +1 on out2, with everything else natural.
- mechanism: 23 sessions' shared frame. out2 needs a fourth flow-counted reference
  (E-s23-5); tbl, i, stptr, stptr2, a4, a3 are at their natural counts in the
  all-target-seat forms; find the byte-free spelling of out2's +1 and the function closes.
- probe: read target's own bytes for out2's ($s6) reference count and live range, then
  measure form P1 = candidate chassis + target's block-2 spelling + exactly one zero-insn
  in-loop1 +1 on out2 (loop-note weight as an instrument), and read the ALLOCDBG table.
- result: out2 lands at 4 refs / live 42 = 1904 and tbl stays at its natural 4 / 47 = 1702,
  so out2 outranks tbl and the two take each other's seats (hardregs 21 / 22 instead of
  22 / 21). The arithmetic depends only on (refs, live length), so NO spelling of a single
  out2 +1 can work while tbl is at four references and an early definition. Target's own
  emitted code fixes out2's live length at 42, so this is a statement about the ORIGINAL
  source, not about our chassis.
- verdict: KILLED

## [s24] The original source gave `tbl` a FIFTH flow-counted reference.
- mechanism: forced by the kill above. out2 = 4 refs / live 42 = 1904 in the original;
  target seats tbl in $s5 above out2 in $s6; tbl's early definition (asm/funcs/func_80041188.s:12-13
  puts its lui/addiu at block-0 insns 3-4) fixes its live length at 47; 4 refs at 47 is 1702,
  below out2. Five references at 47 is 2127, above out2 and below `i` (2474) — the exact
  vector s18's V7 spells and s24 re-measured.
- probe: re-measure V7 on the current chassis, and V8 (V7 with the stptr loop-note wrap
  replaced by candidate.c's F1 chain-extender); read ALLOCDBG + sandbox + fdiff.
- result: CONFIRMED as the required vector — V7 and V8 both produce ALL-TARGET callee-saved
  seats together with target's block-2 `addiu $s3,$s7,0x20`, at sandbox 8 / 133 insns. The
  only other consistent vector is tbl at 4 refs with live <= 41 (Q1's late definition, 1951),
  which E-s22-4 proves emission-costly and structural.
- verdict: CONFIRMED

## [s24] An address-constant split-init (`tbl = D_80094CFC - 1; tbl += 1;`) buys tbl's fifth reference byte-free.
- mechanism: the F1 chain-extender law (E-s22-2) with a donor that dies immediately; combine
  was expected to fold `la BASE` + `addiu off` back into one address constant, exactly as it
  folds candidate.c's `stptr = base; stptr += 0xFC;` into a single addiu.
- probe: T1 = V8 with the tbl loop-note wrap replaced by that split; ALLOCDBG + sandbox.
- result: the references ARE delivered (tbl 6 / 47 = 2553 — the construct is not folded away
  before flow), but (i) it is +2, not +1, and 2553 outranks `i` at 2474 so tbl takes $s4 and
  i takes $s5, and (ii) combine does NOT fold the pair, costing a real instruction:
  134 of 132 build insns, sandbox 19. Both failures are independent of the base symbol
  chosen, so no "tbl is a member of a larger table" reading rescues it.
- verdict: KILLED

## [s24] Frontier reset — the strongest 1-3 hypotheses for the next ladder pass

1. **A byte-free fourth flow-counted reference on out2, sited as a USE inside loop1 that
   combine absorbs — now known to be needed TOGETHER with a byte-free fifth reference on
   tbl, and the tbl half is already solved byte-free.**
   - mechanism: E-s24-5 measured V11 (tbl loop-note wrap alone, no out2 wrap) at 132 of 132
     insns, so the tbl +1 is byte-free on this chassis; the out2 loop-note wrap is what
     materialises the 133rd insn (a load-delay nop, E-s24-4). combine.c's try_combine runs
     after flow.c has fixed reg_n_refs, so a reference living in an insn combine absorbs is
     both counted and byte-free (the shape E-s16-5 proved target's own stptr reference has),
     and E-s23-1 proves a USE is safe where a DEFINITION is not.
   - next probe: work from combine.c's side. Enumerate try_combine 2->1 patterns whose OUTPUT
     is `(set reg (plus reg const))`, `(set reg (mem ...))` or `(set (mem ...) reg)` and whose
     deleted INPUT insn mentions a third register; write the loop1 C that generates that pair
     with out2 as the third register; check build_insns FIRST (a surviving insn is fatal —
     E-s20-2, G1). Candidate consumers are target's own loop1 insns: `addiu $a0,$sp,0x10`,
     the `addu $a1,$s6,$zero` argument moves, `addiu $a2,$s3,0x38`/`addiu $a3,...` for
     func_800523E0, the `sh` into 6($s3), and `addiu $s3,$s3,0x68`. Rule out in advance
     anything cse1 reassociates (E-s20-3) and anything that is a DEFINITION of a value equal
     to a4 + 0x20 (E-s23-1). Do the work on the V8 chassis
     (memory/grind/func_80041188/alt_V8_alltarget_133_s24.c) with the out2 wrap replaced by
     the candidate spelling — every other seat is already target's there.

2. **V8's residual is seven-or-fewer sched1 ORDER diffs plus one load-delay nop, and
   `tools/sched_solver` has still never been run on an all-target-seat form.**
   - mechanism: E-s24-4 attributes the 133rd insn precisely — at index 44 ours emits
     `lhu $v0,0($s0)` + `nop` where target fills the delay slot with `addiu $s4,$s4,1`, and
     target emits `addiu $s5,$s5,4` at index 40 where ours sinks it to 48. Both are sched1
     ranking decisions perturbed by the wraps' loop notes; E-s24-7 shows C-level statement
     order does not move them, which is exactly the regime tools/sched_solver models order-
     and clock-exactly.
   - next probe: apply alt_V8_alltarget_133_s24.c, dump red.i.sched, and run
     tools/sched_solver on loop1 (block 1) against target's order for a typed
     REACHABLE / FORECLOSED verdict plus a ranked C-lever vector. FORECLOSED would prove the
     loop-note delivery of out2's +1 can never be byte-exact regardless of annotation, and
     would leave hypothesis 1 as the only route; REACHABLE names the first concrete levers.

3. **The `i` allocno is the untested third dial: with tbl forced to five references, a
   consistent vector also exists at tbl 6 / 47 = 2553 IF `i` can be lifted from 8 refs to 9
   (2783), which would make the (byte-costly, +2) address-split family on tbl viable if its
   instruction cost can be paid back elsewhere.**
   - mechanism: floor_log2 makes the 8 -> 9 step worth a jump from 2474 to 2783 (the exponent
     goes 3 -> 3 but the count rises, and 9 crosses no boundary downward), so `i` is the one
     allocno whose priority moves by a large step on a single extra reference. No session has
     ever probed `i`'s reference count; s23 proved a3 is rigid, but a3 is a parameter with no
     definition to split while `i` is a real loop counter with an initialiser, an increment,
     a compare and a reset (`i = 0x12`) — four independent split sites.
   - next probe: measure `i`'s reference count response to each of the four sites (split
     initialiser, split increment, split reset, duplicated compare) on the V8 chassis, reading
     ALLOCDBG and build_insns; the question is only whether ANY of them delivers exactly +1
     byte-free. If one does, the whole tbl question reopens at 6 references and the search
     space widens rather than narrows — worth knowing before more effort goes into the +1
     spelling hunt for out2.

## [s24] The residual is a one-reference problem: a byte-free 4th flow-counted reference on out2, with every other allocno at its natural count, closes the function.
- mechanism: The frame shared by s20-s23. out2 needs 4 references (E-s23-5); tbl/i/stptr/stptr2/a4/a3 sit at natural counts in the all-target-seat forms; only out2's +1 spelling was thought to be open.
- probe: Read target's own $s6 references (asm/funcs/func_80041188.s:25,56,61 - three emitted, last one inside loop1, so live length 42 post-combine), then measure form P1 = candidate chassis + target's block-2 spelling (out3 from pa4) + exactly one zero-insn in-loop1 +1 on out2 (do-while(0) loop-note weight used purely as an instrument), and read the ALLOCDBG table.
- result: out2 = 4 refs / live 42 = pri 1904 -> hardreg 21 ($s5); tbl = 4 / 47 = 1702 -> hardreg 22 ($s6). The seats are SWAPPED with respect to target. The arithmetic depends only on (refs, live length), never on the spelling, so no delivery mechanism for a single out2 +1 can work while tbl is at 4 references with target's early definition.
- verdict: KILLED

## [s24] The original source therefore gave tbl a FIFTH flow-counted reference (5/47 = 2127), and that vector is exactly s18's V7.
- mechanism: reg_n_refs is fixed by flow.c:2081 pre-combine and never recomputed; reg_live_length IS recomputed post-combine (sched.c:5106). Target's $s6 dies inside loop1, so the original's out2 live length is 42 and its priority is 1904 at 4 references. Target seats tbl above out2, and tbl's early definition (asm/funcs/func_80041188.s:12-13 = block-0 insns 3-4) fixes its live length at 47, so 4 refs (1702) is impossible and 5 refs (2127) is required.
- probe: Re-measure V7 and V8 (= V7 with the stptr loop-note wrap replaced by candidate.c's F1 chain-extender) on the current chassis: ALLOCDBG + sandbox + fdiff against target.
- result: Both reproduce ALL-TARGET callee-saved seats WITH target's block-2 addiu $s3,$s7,0x20: stptr 3414 $s3, stptr2 2500 $s0, i 2474 $s4, tbl 5/47=2127 $s5, out2 4/42=1904 $s6, a4 1473 $s7, a3 808 $fp. Sandbox 8 at 133 of 132 insns. The only other consistent vector is tbl at 4 refs with live <= 41 (Q1's late definition, 1951), which E-s22-4 already proved emission-costly and structural.
- verdict: CONFIRMED

## [s24] An honest address-constant split-init on tbl (tbl = D_80094CFC - 1; tbl += 1;) buys the required fifth reference byte-free, the way candidate.c's stptr chain-extender does.
- mechanism: The F1 chain-extender law (E-s22-2): a donor that dies at the split point keeps the recipient canonical through cse1, flow counts the extra reference, and combine folds the pair back into one instruction. The value is identical and no new symbol is needed.
- probe: T1 = V8 with the tbl loop-note wrap replaced by that split; ALLOCDBG + sandbox + build_insns.
- result: The references ARE delivered (tbl 6 refs / 47 = 2553, so the construct is not folded away before flow), but it fails twice: it delivers +2 rather than +1, and 2553 outranks i (8/97 = 2474) so tbl takes $s4 and i takes $s5; and combine does NOT fold `la BASE` + `addiu off` back into one address constant, costing a real instruction - 134 of 132 build insns, sandbox 19. Both failures are independent of the base symbol chosen.
- verdict: KILLED

## [s24] The 133rd instruction of the all-target-seat form is a structural cost of the wraps, and moving i++ in the C source to where target schedules it recovers the slot.
- mechanism: fdiff shows ours emits `lhu $v0,0($s0)` + `nop` at index 44-45 where target fills the load-delay slot with `addiu $s4,$s4,1`; if sched1's choice tracked C statement order, writing i++ immediately after the load would restore it.
- probe: V14 = V8 with i++ moved to sit immediately after buf[0] = p[0]; sandbox + build_insns.
- result: Byte-identical to V8 (sandbox 8, 133 insns). sched1 owns the delay slot; the C-level position of the increment inside the block does not move it. Separately, V11 (tbl wrap kept, out2 wrap removed) measures 132 of 132 insns, which attributes the nop to the out2 +1 delivery and proves the tbl +1 is byte-free.
- verdict: KILLED

# ============================ s25 (escalation) ============================

## [s25] tools/sched_solver, run for the first time on an ALL-TARGET-SEAT form, will name a spellable lever for V8's loop1 emission-order residual.
- mechanism: s24 attributed V8's 133rd instruction to two sched1 ranking decisions inside loop1
  (i++ displaced out of a load-delay slot, tbl++ sunk eight slots) and E-s24-7 showed C statement
  order does not move them — exactly the regime tools/sched_solver models order- and clock-exactly.
- probe: apply alt_V8_alltarget_133_s24.c, extract the reduced-TU model (s15 shims), hand-build
  target's block-1 goal order in our UIDs (the automatic goalmap alignment is INVALID here — it
  mis-pairs the two identical `sh $2,20($sp)` texts and reports 4 dependence violations), then run
  perturb.py --pass 1 --block 1 --atoms luid,luid_move.
- result: at depth 1, 2340 single atoms, NONE reaches the goal. Depth 2 did not converge in ~40
  minutes and was stopped. The scheduler's own model therefore agrees with E-s24-7: the wrap-borne
  order damage is not a statement-order problem. The productive reading is that the wraps must GO,
  not be re-positioned.
- verdict: KILLED (at depth 1; depth 2 unresolved and low-value, since E-s25-3 removes the wraps
  outright)

## [s25] A byte-free reference lift is available at the INCREMENT site of a loop-carried variable, where every block-0 chain-extender spelling folds.
- mechanism: the F1 chain-extender law (E-s22-2/E-s17-1) says flow.c:2081 counts reg_n_refs before
  combine and combine folds the pair back. At a redefinition of a loop-carried pseudo the value is
  not available in cse1's table (the reg is live-in from the latch), so cse cannot fold the two
  arithmetic insns into one, and combine — which runs after flow — does.
- probe: rewrite `i++;` as `i += 2; i -= 1;` and `tbl++;` as `tbl += 2; tbl -= 1;` on the V8
  chassis; read ALLOCDBG and the emitted instruction count; then measure the combined form.
- result: CONFIRMED and byte-free. i 8/97 = 2474 -> 10/97 = 3092; tbl 4/47 = 1702 -> 6/47 = 2553;
  no emitted instruction changes. With both applied and BOTH do-while(0) wraps deleted, form V15a
  measures sandbox 15 at 132 of 132 insns with goalmap reporting GOAL == OURS (identity) in all
  four blocks — target's emission order everywhere. This is the first delivery of tbl's extra
  references that costs nothing (s24's T1 cost an insn) and the first 132-insn form with target's
  order and no wrap.
- verdict: CONFIRMED

## [s25] `i`'s reference count is a dial at all four of its split sites.
- mechanism: s24 frontier item 3 — floor_log2 gives i a large single-step jump at 8 -> 9 refs, and
  i has four independent split sites (initialiser, increment, compare, the i = 0x12 reset).
- probe: measure each site separately on the V8 chassis with ALLOCDBG.
- result: KILLED for three of the four. Split initialiser: 8 refs (live 97 -> 96). Split reset
  (`i = 0x11; i++;`): 8 refs, otherwise identical to V8. Duplicated compare: 8 refs — jump.c folds
  the second test. Only the in-loop increment moves the count, and it moves it by +2 (8 -> 10),
  never +1. So the "+1 on i" vector s24 hoped for does not exist; the +2 vector does, and it is
  free.
- verdict: KILLED (three sites); the increment site is CONFIRMED as a +2-only dial

## [s25] A block-0 chain-extender on `out2` with a donor that dies early lifts out2 to four references.
- mechanism: E-s17's law says a chain `X = Y; X += K;` lifts reg_n_refs(X) iff cse1's make_regs_eqv
  (cse.c:844-857) keeps X canonical, which requires X's last use to fall after Y's. V17's donor was
  pa4, whose last use is in loop2 (after out2's), so it folded — but a FRESH donor dying in block 0
  satisfies the condition.
- probe: six spellings measured on two chassis — out2 = pa4 then +0x20; out2 = pa4+0x21 then -1;
  tmp = pa4, out2 = tmp, out2 += 0x20; tmp = pa4+0x10, out2 = tmp, out2 += 0x10; out2 = pa4+0x20
  then +0x20 then -0x20; and the same on the V15a chassis.
- result: KILLED. All six leave out2 at exactly 3 refs / live 42 = 714. The law's condition is
  NECESSARY BUT NOT SUFFICIENT: cse1 folds any constant-offset chain rooted in a pseudo already in
  its table at that point, regardless of the donor's death point. The lifts that DO survive in this
  function have a donor that is a memory-load result used again later (`base`, for stptr) or the
  loop-carried variable itself (tbl, i).
- verdict: KILLED

## [s25] Frontier reset — the strongest hypotheses for the next ladder pass

1. **On the V15a chassis the whole function is ONE byte-free flow-counted reference on `out2`,
   and the only surviving reference-lift MECHANISM in this function is "a donor whose value cse1
   cannot fold at that point".**
   - mechanism: E-s25-5 proves V15a + one out2 reference = target's COMPLETE callee-saved
     disposition WITH target's block-2 `addiu $s3,$s7,0x20` and target's emission order in all four
     blocks; the only cost is the instrument (a do-while(0) wrap, 134 vs 132 insns). E-s25-7 says
     block-0 constant-offset chains all fold; E-s25-3 says a redefinition of a loop-carried pseudo
     does not.
   - next probe: make `out2` loop-carried. On the V15a chassis, replace the invariant `out2` with a
     pointer that is advanced and restored across loop1 (e.g. carried as `out2` with a real
     per-iteration recomputation from a loop-carried base), or find a donor whose value is not in
     cse1's table at out2's definition — the only two candidates in block 0 are `base` (the
     `D_800A9A10[a0]` load result, already spent on stptr) and `saved`. Measure `out2 = saved - K;`
     style derivations: `saved` is a load-derived value used again later (`stptr2 = saved + 0x750`
     in block 2), which is exactly the shape that works for stptr. Check ALLOCDBG for
     out2 == 4 refs AND build_insns == 132 in the same measurement; anything that materialises an
     instruction is fatal.

2. **The V15a chassis makes `out2`'s LIVE LENGTH a second, previously-unusable dial, because tbl
   now sits at 2553 instead of 1702.**
   - mechanism: with tbl lifted to 6/47 = 2553 the admissible band for out2 widens from
     (1473, 1702) to (1473, 2553). At 3 references pri = 30000/live, so out2 is seatable at
     live 12..20 — a window that did not exist in any earlier session's arithmetic (s23 computed
     the 3-reference requirement as live <= 37 against a3 and <= 23 against a4 with tbl pinned low).
   - next probe: enumerate block-0 positions and formulations for out2's definition that shorten its
     live range below 21 WITHOUT moving the definition into loop1 (which E-s23-1's cse-EBB law
     forbids, since it makes cse1 rewrite block 2's `out3 = pa4 + 0x20` into `out3 = out2`). The
     obvious candidate is defining out2 as the LAST statement of block 0 combined with a later
     first use; measure live length directly from ALLOCDBG rather than reasoning about it.

3. **The s25 increment-split construct needs a family ruling before it can ever ship, and that
   question should be settled before a session spends effort spelling a form around it.**
   - mechanism: `i += 2; i -= 1;` is byte-free and its ONLY effect is reg_n_refs. It is not a
     dead store (both statements are live arithmetic on a live variable), not a duplicated statement
     into arms, not a named intermediate, and not the F1 combine-foldable chain-extender as that
     family is described (which is a copy plus a constant add on a FRESH value, not a split of an
     existing increment). No SOTN-master precedent has been searched for it.
   - next probe: grep `docs/reference/sotn-construct-index.md` for split-increment forms
     (`+= N; -= M;` on an induction variable) and, if absent, emit a `ruling-request` naming the
     construct precisely — BEFORE building a candidate on it. If the ruling is NO, V15a's tbl and i
     lifts must be re-derived from an honest structure (loop.c's giv on a real loop, s19's W4
     chassis, is the only known honest source of extra references in this function).

## [s25] tools/sched_solver, run for the first time on an ALL-TARGET-SEAT form, will name a spellable lever for V8's loop1 emission-order residual.
- mechanism: s24 attributed V8's 133rd instruction to two sched1 ranking decisions inside loop1 (i++ displaced out of a load-delay slot, tbl++ sunk eight slots); E-s24-7 showed C statement order does not move them - the regime sched_solver models order- and clock-exactly.
- probe: Applied alt_V8_alltarget_133_s24.c (re-measured: sandbox 8, 132 target / 133 build), extracted the reduced-TU model via the s15 shims, hand-built target's block-1 goal order in our RTL UIDs (the automatic goalmap alignment is INVALID here - it mis-pairs the two identical 'sh $2,20($sp)' texts and reports 4 dependence violations), then ran perturb.py --pass 1 --block 1 --atoms luid,luid_move --depth 1.
- result: 2340 single atoms searched, NONE reaches the goal. The depth-2 search did not converge in ~40 minutes and was stopped before the turn ended (no orphan). The scheduler's own model therefore agrees with E-s24-7: the wrap-borne order damage is not a statement-order problem, so the wraps must be removed rather than re-positioned.
- verdict: KILLED

## [s25] A byte-free reference lift is available at the INCREMENT site of a loop-carried variable, where every block-0 chain-extender spelling folds.
- mechanism: flow.c:2081 counts reg_n_refs before combine and combine folds the arithmetic pair back into one insn (the F1 law, E-s22-2/E-s17-1). At a redefinition of a loop-carried pseudo the value is live-in from the latch and is not in cse1's table, so cse1 cannot fold the split first.
- probe: Rewrote 'i++;' as 'i += 2; i -= 1;' and 'tbl++;' as 'tbl += 2; tbl -= 1;' on the V8 chassis; read ALLOCDBG and the emitted instruction count; then measured the combined form (V15a) with sandbox and goalmap.
- result: CONFIRMED and byte-free. i 8 refs/97 = 2474 -> 10/97 = 3092; tbl 4/47 = 1702 -> 6/47 = 2553; no emitted instruction changes. With both applied and BOTH do-while(0) wraps deleted, V15a = sandbox 15 at 132 build / 132 target insns, and goalmap reports GOAL == OURS (identity) in blocks 0, 1, 2 and 3 (hon=131 tgt=131). First zero-cost delivery of tbl's extra references (s24's T1 cost an instruction) and the first 132-insn form carrying target's emission order with no wrap.
- verdict: CONFIRMED

## [s25] The V15a chassis plus exactly one more flow-counted reference on out2 produces target's COMPLETE callee-saved disposition.
- mechanism: V15a's residual is the {out2, pa4, a3} 3-cycle: out2 sits at 3 refs / live 42 = 714, below a3's 808 and pa4's 1473. At 4 refs / live 42 it is 1904, inside the admissible band (1473, 2553) that tbl's new 2553 opened.
- probe: V23 = V15a with the loop1 func_8004A348(buf, out2) call wrapped in do { } while (0) purely as a reference-count instrument; read ALLOCDBG and sandbox.
- result: CONFIRMED. ALLOCDBG: stptr 3414 -> $s3, i 3092 -> $s4, tbl 2553 -> $s5, stptr2 2500 -> $s0, out2 4/42 = 1904 -> $s6, pa4 1473 -> $s7, a3 808 -> $fp, out3 -> $s3 = ALL-TARGET seats, and this chassis derives out3 from pa4 so it carries target's block-2 'addiu $s3,$s7,0x20' rather than candidate.c's residual 'move $s3,$s6'. The only cost is the instrument: sandbox 9 at 134 insns (the loop note costs two instructions on this chassis, not V8's one).
- verdict: CONFIRMED

## [s25] i's reference count is a dial at all four of its split sites (s24 frontier item 3).
- mechanism: floor_log2(refs)*refs/live gives i a large single-step jump at 8 -> 9 references, and i has four independent split sites: initialiser, increment, compare and the i = 0x12 reset.
- probe: Measured each site separately on the V8 chassis with ALLOCDBG: s32 i = 0 plus i++ in block 0; i = 0x11 plus i++ at the reset; a duplicated compare if (i < 0x12) { if (i < 0x12) goto loop1; }; and the in-loop increment split.
- result: KILLED for three of the four. Split initialiser leaves i at 8 refs (live 97 -> 96, pri 2500); split reset leaves it at 8 refs and is otherwise identical to V8; the duplicated compare leaves it at 8 refs because jump.c folds the second test. Only the in-loop increment moves the count, and it moves it by +2 (8 -> 10), never the +1 s24 hoped for.
- verdict: KILLED

## [s25] A block-0 chain-extender on out2 with a donor that dies early lifts out2 to four references.
- mechanism: E-s17's law: a chain X = Y; X += K lifts reg_n_refs(X) iff cse1's make_regs_eqv (cse.c:844-857) keeps X canonical, which requires X's last use to fall after Y's. V17's donor pa4 has its last use in loop2, after out2's - but a FRESH donor dying in block 0 satisfies the condition.
- probe: Six spellings measured across two chassis: out2 = pa4 then +0x20 (on the V8 and the V15a chassis); out2 = pa4 + 0x21 then -1; tmp = pa4, out2 = tmp, out2 += 0x20; tmp = pa4 + 0x10, out2 = tmp, out2 += 0x10; out2 = pa4 + 0x20 then +0x20 then -0x20.
- result: KILLED. All six leave out2 at exactly 3 refs / live 42 = 714 (the last only shortens live to 41). The E-s17 condition is NECESSARY BUT NOT SUFFICIENT: cse1 folds any constant-offset chain rooted in a pseudo already in its table at that point, regardless of when the donor dies. The lifts that survive in this function all have a donor that is a memory-load result used again later (base, for stptr) or the loop-carried variable itself (tbl, i).
- verdict: KILLED

## s26 (2026-08-27) — hypotheses

### H-s26-1 — CONFIRMED. A real loop construct is an honest source of flow.c's loop_depth reference weighting.
**Statement.** The +1-per-in-loop-reference dial that the layer-1-FAILed `do { } while (0);` wrap
was exploiting is not a property of that wrap; it is a property of NOTE_INSN_LOOP_BEG/END, which
the C front end emits for ANY real loop construct. Writing loop1 as an ordinary
`do { … } while (i < 0x12);` therefore delivers the same dial with no coercion construct at all.
**Probe.** Form Y2 = V15a with loop1 converted to a real do-while and the two FAKE increment
splits REMOVED; ALLOCDBG + sandbox.
**Result.** out2 3 refs/live 42 = 714 → 5 refs/live 41 = 2439; 132 build insns;
`sandbox --disable all` = 13 (V15a, with its FAKE splits, was 15). Every in-loop reference of
every allocno doubled, exactly as the loop_depth model predicts.
**Verdict.** CONFIRMED. See E-s26-1.

### H-s26-2 — CONFIRMED. On the real-loop chassis the F1 stptr chain extender is dead weight, so the chassis is FAKE-free.
**Statement.** `stptr = base; stptr += 0xFC;` (the one construct in the candidate lineage needing a
`/* FAKE */` annotation) buys nothing once loop.c strength-reduces the stptr biv.
**Probe.** Form Z1 = Y2 with `stptr = base + 0xFC;`.
**Result.** Bit-identical to Y2: same allocno table, 132 insns, sandbox 13.
**Verdict.** CONFIRMED. See E-s26-2. Banked as `alt_Z1_realloop_honest_s26.c`.

### H-s26-3 — KILLED. A no-op statement can lift a reference count.
**Statement.** If `i += 2; i -= 1;` lifts i by +2 byte-free, a simpler value-preserving statement
(`out2 = out2;`, `out2 += 0;`) should lift out2 the same way.
**Probe.** W1/W2/W3/W4 on the V15a chassis (loop1 body, loop1 top, block 0), ALLOCDBG each.
**Result.** All four inert — out2 rigid at 3 refs / live 42 / pri 714, 142 insns, allocno table
identical to V15a.
**Verdict.** KILLED. The lifting statement must produce a genuinely NEW value at RTL expansion;
a statement whose RHS is already the LHS's value never becomes an insn. See E-s26-5. This also
rules the dead-store family out as a delivery vehicle for this particular residual.

### H-s26-4 — KILLED. Relocating out2's definition into loop1 puts its priority in the admissible band.
**Statement.** Moving the definition shortens out2's live range and (on the goto chassis) adds a
reference, so some position should land pri inside (1473, 2500).
**Probe.** X1/X2 (goto chassis, two positions) and Z3 (real-loop chassis).
**Result.** Goto chassis: out2 → 4 refs / live 21 / pri 3809, seizes $s3, and the build falls to
141 insns because target's block-0 `addiu $s6,$s7,0x20` no longer exists in block 0. Real-loop
chassis: LICM hoists the definition back to the preheader; bit-identical to Y2.
**Verdict.** KILLED on both chassis. See E-s26-7.

### H-s26-5 — KILLED. Splitting pa4's loop1 uses into a second local lowers pa4 under out2 for free.
**Statement.** pa4 at 9 refs/94 = 2872 outranks out2 at 5/41 = 2439; moving loop1's two pa4 uses
to a copy `pm` drops pa4 to 6/95 = 1263 and should hand out2 the $s6 seat.
**Probe.** Form Z4, ALLOCDBG + insn count.
**Result.** The seat arithmetic works exactly as predicted (out2 → $s6, pm → $s7), but a3 loses
its hard register entirely (hardreg=-1) and the build grows to 143 insns.
**Verdict.** KILLED. See E-s26-9.

### H-s26-6 — CONFIRMED (control). The base-rooted chain extender is a real dial on the goto chassis.
**Probe.** X3 = V15a with `stptr = base + 0xFC;`.
**Result.** stptr 7 refs/41 = 3414 → 5/41 = 2439 and loses $s3.
**Verdict.** CONFIRMED — so E-s26-2's null result is a property of the real-loop chassis, not a
measurement error, and E-s25-7's cse1-foldability law stands.

## [s26] A real loop CONSTRUCT is an honest source of the +1-per-in-loop-reference dial that the layer-1-FAILed do-while(0) wrap was exploiting, because GCC 2.7.2's flow.c increments REG_N_REFS by loop_depth and loop_depth is driven by NOTE_INSN_LOOP_BEG/END, which expand_start_loop emits only for while/for/do — never for a goto loop.
- mechanism: flow.c ref counting weighted by loop_depth; the notes come from the front end's expand_start_loop, so every allocno in this function has been ref-starved on every goto chassis this grind has ever built.
- probe: Form Y2 = V15a with loop1 converted to `do { ... } while (i < 0x12);` AND the two FAKE increment splits REMOVED. BB2_ALLOC_DEBUG cc1 dump + sandbox --disable all.
- result: out2 3 refs / live 42 = 714 -> 5 refs / live 41 = 2439; i 10/97 -> 11/97; tbl 6/47 -> 7/47; pa4 7/95 -> 9/94 = 2872; a3 4/99 -> 5/99. 132 build / 132 target insns, sandbox 13 (V15a WITH its two FAKE splits was 15).
- verdict: CONFIRMED

## [s26] On the real-loop chassis the F1 stptr chain extender (`stptr = base; stptr += 0xFC;`) — the one construct in the candidate lineage that required a /* FAKE */ annotation — is dead weight, making the whole chassis FAKE-free.
- mechanism: loop.c strength-reduces the stptr biv away before its reference count can matter to global.c, so the extra references the extender buys are never spent.
- probe: Form Z1 = Y2 with the plain `stptr = base + 0xFC;`. ALLOCDBG + sandbox.
- result: Bit-identical to Y2: same allocno table, 132 build insns, sandbox 13. Z1 contains no FAKE construct of any kind. Banked as memory/grind/func_80041188/alt_Z1_realloop_honest_s26.c.
- verdict: CONFIRMED

## [s26] A value-preserving no-op statement (`out2 = out2;` / `out2 += 0;`) lifts out2's flow-counted reference count the way `i += 2; i -= 1;` lifts i.
- mechanism: If flow.c counts textual references, any extra statement naming out2 should count, regardless of whether it computes a new value.
- probe: Four spellings on the V15a chassis — W1 (self-assign immediately before loop1's out2 call), W2 (`out2 += 0;` same site), W3 (self-assign in block 0 after the definition), W4 (self-assign at the top of loop1). ALLOCDBG each.
- result: All four completely inert: out2 rigid at 3 refs / live 42 / pri 714, 142 insns, allocno table identical to V15a's. The lifting statement must produce a genuinely NEW value at RTL expansion; a statement whose RHS is already the LHS's value never becomes an insn, so flow.c has nothing to count. This also disposes of the dead-store family as a delivery vehicle for THIS residual.
- verdict: KILLED

## [s26] Relocating out2's definition into loop1 lands its priority inside the admissible (1473, 2500) band.
- mechanism: Moving the definition shortens out2's live range, and pri = floor_log2(refs)*refs*10000/live, so some position should land in band.
- probe: X1 and X2 on the goto chassis (definition immediately before out2's first use, and at the top of loop1); Z3 on the real-loop chassis. ALLOCDBG + insn count each.
- result: Goto chassis: out2 does reach 4 refs but live collapses to 21 -> pri 3809, far above the band, and it seizes $s3; the build also falls to 141 insns because target's block-0 `addiu $s6,$s7,0x20` ceases to exist in block 0. Real-loop chassis: LICM hoists the definition straight back to the preheader, bit-identical to Y2 — out2's definition POSITION is not a dial there at all.
- verdict: KILLED

## [s26] Splitting pa4's two loop1 uses into a second local (`pm = pa4;`) lowers pa4 under out2 for free and hands out2 the $s6 seat.
- mechanism: pa4's in-loop1 references cost 2 each under loop_depth weighting; moving them to a copy drops pa4 from 9 refs/94 = 2872 to 6/95 = 1263, below out2's 2439.
- probe: Form Z4 on the real-loop chassis. ALLOCDBG + insn count.
- result: The seat arithmetic works exactly as predicted — out2 (5/41 = 2439) takes $s6 and pm takes $s7 — but a3 loses its hard register entirely (hardreg=-1) and the build grows to 143 insns.
- verdict: KILLED

## [s26] The block-0 chain extender on out2 folds because of a property of the goto chassis rather than a property of cse1.
- mechanism: E-s25-7 measured six spellings on two goto chassis; the real-loop chassis re-lays cse1's EBB structure, so the fold might not survive it.
- probe: Form Z2 = Z1 with `out2 = pa4; out2 = (s32 *)((u8 *)out2 + 0x20);`. ALLOCDBG.
- result: out2 rigid at 5 refs / live 41, bit-identical to Z1/Y2. cse1 folds a constant-offset chain rooted in a pseudo already in its table regardless of chassis. E-s25-7's law is chassis-independent. (Control X3 confirms the asymmetry is real: dropping the base-rooted stptr extender on the goto chassis DOES cost stptr 7 refs/41 = 3414 -> 5/41 = 2439 and its $s3 seat.)
- verdict: KILLED
