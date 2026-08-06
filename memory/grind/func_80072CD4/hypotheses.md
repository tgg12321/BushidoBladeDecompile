# Hypothesis ledger — func_80072CD4

## s1 (recon) — live frontier

### H1 [CONFIRMED mechanism, partial] — distinct-literal defeats CSE re-mat
Target emits an EXTRA `0xFC` re-materialization at @0x14 (in $v0) that clean-C folds away.
Writing @0x14 as a distinct `0xFC` literal (not the shared fc_const holding @4/@0xC) forced
GCC to re-materialize: build_insns 77→78, score 20→18. Pure C, not a cheat.
NEXT: close the last insn (78→79). Candidate levers: also split @4 vs @0xC into distinct
literals; or reorder so @0xE (var_v0) lands exactly between the @0xC and @0x14 stores as target;
objdump-diff the 78-insn build against target to name the exact missing insn.

### H2 [OPEN] — $v0/$v1 rotation inverted (the dominant residual)
Target carries var_v0 in $v0 and 0xFC in $v1; our build does the reverse. This is the bulk of
the score-18 residual after the insn-count is nearly closed. NEXT (register-alloc-pure-c Step-0):
`cc1 <flags> -da base.i` greg dump on the 78-insn form; read allocno priority tiebreak pinning
var_v0. Levers to try: order var_v0's births in the arm delay slots (target sets 0x32 in THEN
`j` delay slot, 0x46 at ELSE tail) — mirror by making var_v0 the LAST assignment in each arm;
narrow/wide type on fc_const vs var_v0; sibling 72BC4 greg diff.

### H3 [OPEN] — is 12 even a legitimate floor?
The imported floor=12 depends on the reviewer-FAIL empty-do-while barrier. The best LEGITIMATE
pure-C form measured is 18. If H1+H2 cannot drive the clean form below ~16, this is a candidate
for the endgame-lock / do-while-zero sanctioned-exception evaluation (needs documented
lever-exhaustion + the reorg.c LABEL_OUTSIDE_LOOP_P mechanism check — the beqz here is EQ, so
verify the do-while sanction's NE-invert-peephole prerequisite actually applies before proposing).

## [s1] Writing @0x14 as a distinct 0xFC literal (not the shared fc_const holding @4/@0xC) forces GCC to re-materialize the constant, adding the target's extra insn.
- mechanism: Shared fc_const gets CSE-folded into one $v1; distinct literals defeat the fold. Target re-materializes 0xFC in $v0 at @0x14 because the @0xE var_v0 store splits the fc_const uses.
- probe: sandbox --disable all on clean sibling-mirror form: @0x14=fc_const -> 20 (build_insns 77); @0x14=distinct 0xFC literal -> 18 (build_insns 78).
- result: 77->78 insns, score 20->18; one 0xFC re-materialization recovered, still 1 insn short of 79.
- verdict: CONFIRMED

## [s1] Clean sibling-mirror (int fc_const hoisted, shared for all 0xFC stores, no do-while barriers) closes func_80072CD4.
- mechanism: Mirror matched sibling func_80072BC4 which byte-matches via hoisted int fc_const.
- probe: sandbox --disable all = 20, build_insns 77 (2 short) — CSE folds all 0xFC into one $v1, target's re-mat lost.
- result: score 20, farther than HEAD's 12; folded away the target re-mat.
- verdict: KILLED

## [s2] Writing @0xE per-arm with its real branch value (0x32/0x46) instead of via a cross-block temp used in the merge fixes the $v0/$v1 rotation and closes the 1-insn gap.
- mechanism: greg dump: at floor 18, var_v0 (pseudo 74) took $v1 and fc_const (76) took $a0 because both conflict with $v0 (byte constants 0xC3/0x1E/0xC8 occupy $v0 while the sched1-hoisted var_v0 li is live). Per-arm @0xE gives the value a short arm-local range (li pinned by its own sb, no hoist, no $v0 conflict); cross-jump merges the identical `sb v0,0xE` to the merge point, leaving `li v0,X` in each arm delay slot = target shape. RA becomes fc->$v1, @0xE->$v0.
- probe: Rewrite @0xE into each arm as a direct per-branch literal; sandbox --disable all.
- result: 18 -> 9, build_insns 78 -> 79.
- verdict: CONFIRMED

## [s2] Ordering the arg0>=4 branch stores as target (@4 first -> outer-beqz delay li v0,0x10; @5,@6 next -> multi-use v1/a0 consts hoisted; then @0xC) closes 5 of the residual.
- mechanism: @4 first feeds the outer-beqz delay slot with li v0,0x10 as in target; @5(v1),@6(a0) earliest LUID -> sched2 hoists their li's ahead of sb v0,4; @0xC follows.
- probe: Reorder the arg0>=4 store sequence; sandbox after each move.
- result: 9 -> 5 -> 4 -> ... (the else-branch order accounts for ~5 of the drop from 9).
- verdict: CONFIRMED

## [s2] A fully-natural (non-duplicated) form cannot beat 4 because a cross-block var_v0 (target's shape) is hoisted by sched1 into a $v0 conflict.
- mechanism: base.i.sched shows sched1 moves var_v0's `li` (insn 60) to the arm TOP, ahead of byte-stores 45/50/55 (emitted-order 60>57), so var_v0 lives across the $v0 byte constants and is forced to $v1 (score 13, build 78). The only in-session pin that keeps the li at the arm tail (per-arm @0xE sb) glues that sb to the cross-jump merge head, which defers @4/@0xC (the residual-4 store-order diff).
- probe: Cross-block var_v0 form + arg0>=4 fix, sandbox + greg/sched dump.
- result: 13 (build 78). Confirms the hoist is the blocker; per-arm @0xE (4) is the best clean structure this session.
- verdict: CONFIRMED

## [s2] Duplicating @4=fc_const and @0xC=fc_const into both inner arms reaches sandbox 0 but is a cheat, not a valid clean match.
- mechanism: jump2 cross-jump merges the second copy away (byte-neutral); the copy is dead in emitted output and its only effect is steering the merge store schedule. Cheat-by-spelling / duplicated-statement-into-arms family used for store-order (outside that carve-out's RA-priority scope), missing FAKE annotation + documented exhaustion. Layer-1 cheat-reviewer verdict FAIL.
- probe: Add the duplication; sandbox=0; dispatch cheat-reviewer subagent.
- result: sandbox 0, build 79, but reviewer FAIL. Banked rejected/dup4_0xc_into_arms.c.
- verdict: KILLED

## [s2] Priority-reweighting levers do NOT flip the rotation on the score-18 base.
- mechanism: The rotation is driven by var_v0's $v0 conflict (sched1 hoist), not fc-vs-var priority ordering.
- probe: decl-order swap; var_v0 s32; var_v0-early; @0xE-last; fc-before-outer-if; each sandboxed.
- result: decl-order=18, s32=18 (no change); var_v0-early=19; @0xE-last=19; fc-before-outer-if=19.
- verdict: KILLED

## [s3] The cross-block var_v0 form's sched1 hoist is NOT defeatable by any pure structural lever.
- mechanism: Bottom-up list scheduler gives each byte-store li a 7f000001 launch boost via its own
  in-arm sb, pinning it in source order. var_v0's lone constant li has no in-arm consumer (its @0xE
  use is cross-block), so it never earns the boost, loses the potential_hazard tiebreak to the stores,
  and is placed first (hoisted to arm top) -> live across the $v0 byte constants -> RA forces it to $v1.
- probe: cc1 -da sched1 + greg dumps (tmp/grind/func_80072CD4/s3); measured variants: int type (13),
  @0xE-first-in-merge (13), arm-reorder+var-first (17). Enumerated arm values -> no byte-neutral
  in-arm value-reuse consumer exists (all distinct constants != var_v0).
- result: all cross-block structural variants >= 13; per-arm stays 4. Structural axis exhausted.
- verdict: KILLED

## [s3] The per-arm floor-4 form cannot reach target store order (@4,@0xC before @0xE) in clean C.
- mechanism: cross-jump (jump2) merges the identical per-arm `sb v0,0xE` to the merge HEAD (shared
  tail inserted at the join label), so @0xE always precedes the native merge stores @4/@0xC. Putting
  @4/@0xC before it requires them in the arms (duplication) — the rejected dup4 cheat.
- probe: objdump of the score-4 build vs target (tmp/grind/func_80072CD4/s3); sched2 defers @4/@0xC
  because their $v1 data is available early (no launch pairing).
- result: residual 4 is structurally locked to the cross-jump merge point.
- verdict: KILLED (structural); 0-path is the owner-ruling duplication question only.

## [s3] The clean cross-block var_v0 form (target's own structure: @0xE a native merge store, correct @4,@0xC-before-@0xE order) fails only on RA because sched1 hoists var_v0's li; this hoist is defeatable by a structural lever.
- mechanism: sched1 is a bottom-up list scheduler. Each byte-store li earns a 7f000001 launch priority from its own in-arm sb and is pinned in source order (sched.c launch mechanism). var_v0's lone constant li has no in-arm successor (its @0xE use is cross-block), never earns the boost, loses the potential_hazard tiebreak to the stores (sched.c:2683-2699, sb memory-unit > li), and is picked last bottom-up = placed first (hoisted to arm top). Hoisted => var_v0 live across the 3 $v0 byte constants => $v0 conflict => RA forces var_v0->$v1, fc->$a0 (greg reg75 in 3, reg74 in 4). build 78, score 13.
- probe: cc1 -da sched1 + greg dumps on the cross-block form; measured 4 structural variants: var_v0 int/SI (13), @0xE first in merge (13), arm byte-store reorder + var-first (17), plus enumeration of arm values for a byte-neutral in-arm consumer.
- result: All cross-block structural variants >= 13 (per-arm stays 4). @0xE merge-position is irrelevant (conflict is in the arm). No byte-neutral in-arm consumer exists: every arm value (0xC3/0x1E/0xC8/0x50/0xDC) is a distinct constant != var_v0 (0x32/0x46), and any arithmetic derivation from the loaded flag emits andi/mul/subu (structurally != target's per-branch li).
- verdict: KILLED

## [s3] The per-arm floor-4 form can be reordered to reach target's merge store order (@4,@0xC before @0xE) in clean C.
- mechanism: cross-jump (jump2) merges the identical per-arm `sb v0,0xE` into a shared tail inserted at the join label, so the merged @0xE always sits at the merge HEAD, before the native merge stores @4/@0xC; sched2 further defers @4/@0xC because their $v1 data is available early (no launch pairing).
- probe: objdump of the score-4 build vs target/asm-funcs; per-arm vs cross-block sched-dump diff confirming insn 59 (in-arm sb) is what pins var_v0's li correctly.
- result: Residual 4 is structurally locked to the cross-jump merge point; @4/@0xC before @0xE requires putting them in the arms (the rejected dup4 duplication).
- verdict: KILLED

## [s4] A fresh-seed permuter campaign (both floor-4 and cross-block chassis) finds NO clean C reorder that defeats the sched1 hoist / residual-4 store order; every near-residual closing form it surfaces is a cheat.
- mechanism: The residual is a store-SCHEDULING-order artifact locked by the cross-jump merge point (per-arm chassis) or the sched1 bottom-up hoist of the lone cross-block li (cross-block chassis). Random C mutation cannot re-derive target's schedule without a fence/coercion, so the only sub-residual forms the permuter reaches are scheduling barriers (empty do-while around the arm block) and volatile coercions — both forbidden, and neither even a byte match.
- probe: Two telemetry campaigns. A: floor-4 per-arm base (perm-270==honest-4), 6293 iters/42 finds, best perm-25 = empty do-while barrier (cheat, not a match). B: cross-block base (perm-1605), 4983 iters/481 finds, best perm-308 = volatile char coercion (cheat, not a match). Neither produced a perm-0.
- result: No clean 0 in ~11k combined iters across two structurally-distinct chassis; both best forms are cheats. Search modality exhausted.
- verdict: KILLED (no clean C lever exists in the searched space; the 0-path is the reviewer-FAIL dup form = an owner ruling question).

## [s4b] Directed permuter (PERM_LINESWAP over the merge store block) finds no source ordering that defeats the residual-4 store-order.
- mechanism: residual-4 is a merge store-ORDER artifact: cross-jump (jump2) glues the per-arm `sb v0,0xE` to the merge HEAD and sched2 defers @4/@0xC (their $v1 data is available early), regardless of the source order of the 8 arg0<4 merge stores. Exhaustively permuting that block cannot change the emitted order without a scheduling fence (a cheat).
- probe: DIRECTED permuter, PERM_LINESWAP over the 8-statement merge store block (the exact un-run permuter surface; prior-s4 ran only random full-function mutation). ws_directed, base perm-270 (==honest 4), 15,825 iters.
- result: 1 find (output-270-1) at score 270 == base (score-neutral reorder, not a match, not a cheat); ZERO finds below base. All pure-C search axes (structural s3 + random x2 chassis prior-s4 + directed s4b) exhausted.
- verdict: KILLED (no clean C lever in the directed permutation space either; the only 0-path is the judge-FAILed dup store-schedule cheat = owner ruling).

## [s4] Directed permuter (PERM_LINESWAP over the arg0<4 merge store block) finds no source ordering that defeats the residual-4 store-order.
- mechanism: Residual-4 is a merge store-ORDER artifact: jump2 cross-jump glues the per-arm `sb v0,0xE` to the merge HEAD and sched2 defers @4/@0xC (their $v1 data is ready early), regardless of the source order of the 8 arg0<4 merge stores. Exhaustively permuting that block cannot change emitted order without a scheduling fence (a cheat).
- probe: Built ws_directed (reused floor-4 target.o/harness) with the 8-statement merge store block wrapped in PERM_LINESWAP; launched via tools/permuter_campaign.py (telemetry), base perm-score 270 (== honest 4); waited two in-turn windows (15s + 540s), harvest --stop.
- result: 15,825 iters, ONE find (output-270-1) at score 270 == base (score-neutral reorder, not a match, not a cheat); ZERO finds below base. Directed permuter axis measured dead.
- verdict: KILLED

## [s4] All pure-C search/structural axes are now exhausted; the only sandbox-0 path is the judge-FAILed dup store-schedule cheat, so func_80072CD4 is a confirmed clean-floor-4 endgame-lock.
- mechanism: Structural (s3) dead + random permuter x2 chassis (prior-s4, ~11k iters, only cheat forms) dead + directed permuter (s4b) dead + dup4_0xc_into_arms store-schedule duplication ruled FAIL by the committed 2026-07-24 16:38 judge (outside the duplicated-statement-into-arms reg_n_refs RA-priority scope, no SOTN precedent). Compiled C (not hand-written asm), so canonical-asm is not available.
- probe: Cross-checked the committed judge ruling + full s1-s4 ledger against the endgame-lock-disposition policy; filed the OWNER-ESCALATION entry naming func_80072CD4 in docs/grind/decisions.md.
- result: Every grind-advanceable axis measured/ruled dead; escalation filed; clean floor-4 candidate applied to src (sandbox --disable all = 4, build_insns 79 == target).
- verdict: CONFIRMED

## [s+] The inverse-compose RA verdict (block 5, qty 3 <-> qty 1) names a residual this function's banked candidate has ALREADY solved.
- mechanism: `inverse_compose.py classify text1b func_80072CD4` reports FIRST
  DIVERGENCE: RA, with 252 in $v0 (ours) vs $v1 (target) and 50/70 the other way,
  and routes to the local backend (22 single-atom vectors, cheapest live_shrink
  qty 1 span 14->3 / refs_up qty 1 3->7).
- probe: Checked what baseline the model is built from. `mkasm_honest.sh` compiles
  `engine.inlineasm.write_stripped(<stem>)` — that is MAIN's source with cheat-asm
  stripped. main here carries 9 regfix rules and no cheat-asm, so the honest stream
  is the distance-12 body, NOT the banked distance-4 candidate.
- result: The classifier's $v0<->$v1 constant-holder exchange is precisely the
  diff the s2 candidate already fixed (its header records the 18 -> 9 step as
  "the value stops sharing a live range with the $v0 byte-store constants, so RA
  matches target: fc_const->$v1, @0xE-value->$v0"). So the 22 vectors address a
  solved sub-problem. GENERAL: for any parked function whose banked candidate
  beats main, every ra_solver / sched_solver model is extracted from the wrong
  baseline, and its verdict must be re-derived from the candidate before use.
- verdict: CONFIRMED

## [s+] Live-range / naming perturbations on the cross-block @0xE value keep its `li` in the arm delay slot and reach the target merge store order.
- mechanism: The candidate's own FRONTIER: target writes @0xE once at the merge
  from a cross-block variable, and its `li` sits in the arm's j-delay slot; our
  cross-block form lets sched1 hoist that `li` to the arm top, so the value goes
  live across the $v0 byte constants, conflicts, and lands in $v1. The
  func_80037A20 result this session (a named intermediate moved a pseudo's refs
  and live range enough to flip a global.c allocno order) suggested the same
  family might shape this live range.
- probe: Six variants over the candidate, all with the cross-block `e_val` and a
  single merge-point `sb @0xE`: plain target shape; `e_val` declared before
  `fc_const`; `e_val` assigned FIRST in each arm (longest arm-local range);
  `fc_const` also made cross-block; @4/@0xC fed through their own named locals;
  and @0xE written before @4/@0xC as a control.
- result: KILLED. All six score 13 at 78 insns — identical to the ledger's
  documented cross-block failure, and worse than the banked candidate's 4 at 79.
  Declaration order, arm-local assignment position, extra named merge locals and
  the control ordering are all inert: sched1 hoists the `li` regardless, so the
  conflict that forces $v1 is re-created every time. The naming family that
  worked on func_80037A20 does not transfer here, because there the lever moved a
  pseudo's REFERENCE COUNT (an allocno-priority input) whereas here the binding
  constraint is a scheduler hoist that no naming choice constrains.
- verdict: KILLED
