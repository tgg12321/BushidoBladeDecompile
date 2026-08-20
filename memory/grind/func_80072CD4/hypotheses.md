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

## [s5] The banked floor-4 candidate still reproduces on the post-migration (asm-until-matched) chassis.
- mechanism: The 2026-08-19 migration replaced the 9-rule main body with `INCLUDE_ASM("asm/funcs", func_80072CD4);`, so every banked score is chassis-relative and must be re-measured before it is spent.
- probe: Applied candidate.c over the INCLUDE_ASM line (symbol names refreshed to SetPolyG4 / SetSemiTrans / AddPrim) and ran `sandbox func_80072CD4 --disable all`.
- result: score 4, build_insns 79 == target, rules_dropped 0. Floor unchanged by the migration.
- verdict: CONFIRMED

## [s5] "Lever A" (route the arm byte constants through a shared local so they vacate $v0) lets a cross-block var_v0 claim $v0 and reach target's RA.
- mechanism: 2026-06-14 WIP hypothesis, never measured in s1-s4: the arm constants 0xC3/0x1E/0xC8 occupy $v0 exactly where the sched1-hoisted var_v0 is live; giving them their own pseudo should free $v0 for var_v0 and undo the $v0/$v1 rotation.
- probe: Shared `int t` holder feeding all three arm byte stores, measured on BOTH chassis (per-arm floor-4 base and cross-block base), plus a chained variant (var_v0 fed from t) and an objdump of the cross-block result.
- result: per-arm = 4 (inert; the holder is folded away, 79 insns unchanged). cross-block = 11/78, chained variant = 11/78. NOT a better basin: the shared holder makes the two arms' `sb v0,0xD` stores identical, so jump2 cross-jumps @0xD to the merge head - the build loses the insn target keeps inside each arm (78 vs 79) and the rotation survives (fc -> $a0, var_v0 -> $v1). Lever A cannot fix the RA because target REQUIRES those constants in $v0 (`addiu $v0,$zero,0xC3` ...); vacating $v0 is byte-divergent by construction.
- verdict: KILLED

## [s5] Moving the @4/@0xC stores out of the merge block (above the inner `if`) escapes the cross-jump-imposed store order.
- mechanism: The residual is that the cross-jumped `sb v0,0xE` tail is spliced at the join label ahead of the native merge stores; emitting @4/@0xC earlier in program order should put them ahead of it.
- probe: @4 = fc_const, @0xC = fc_const written immediately after `fc_const = 0xFC;` and before the inner `if`, per-arm chassis.
- result: 8, build_insns 78 - the stores land before the inner branch (target has them at the merge) and the build loses an insn. Strictly worse than the floor.
- verdict: KILLED

## [s5] A different whole-function C SPELLING (base pointer local instead of casted-offset stores) changes the scheduling/RA outcome.
- mechanism: Every s1-s4 form used `*(u8 *)((s32)arg1 + N)`; a `u8 *p = (u8 *)arg1; p[N] = ...` spelling gives GCC a different MEM rtx form and an extra pseudo, which could reshape the merge block.
- probe: Mechanical rewrite of all field writes to `p[N]`; sandbox.
- result: 17, build_insns 82 (the pointer local costs a copy). Spelling axis is negative.
- verdict: KILLED

## [s5] SYNTHESIS FRONTIER RESET - the function is a two-attractor lock, and the sanctioned axes are empty.
- mechanism: (a) per-arm @0xE: 4/79, correct arms + correct RA, wrong merge store order, fixed by jump2's splice of the cross-jumped tail at the JOIN LABEL (source-order-invariant, proven by the s4b directed permuter over the full 8-store permutation space). (b) cross-block var_v0: 13/78 (11/78 with Lever A), target's source shape, correct order, wrong RA from the sched1 hoist of the launch-boost-less lone `li` (sched.c:2683-2699). No lever in 5 sessions produced a third attractor; the only sandbox-0 form is the store-schedule duplication FAILed by the Judge (2026-07-24 16:38) and refused by the owner (2026-07-27).
- probe: s5 re-measured the chassis, re-derived the residual from objdump, and closed the last three never-run levers (Lever A x2 chassis, @4/@0xC pre-branch hoist, base-pointer spelling).
- result: All sanctioned axes measured dead on the CURRENT chassis. Nothing grindable remains that is not a re-run of a banked kill.
- verdict: CONFIRMED


## [s5b] SYNTHESIS FRONTIER RESET (2026-08-20, respawn after the s5 owner-gated discard) � the residual is a SEMANTIC-MODEL bug, not a scheduler wall: arg1 is a PSX libgpu POLY_G4 and its offsets are four RGB triples.
- statement: Every s1-s5 form treated `arg1` as an opaque byte blob and asked which *ordering*
  of independent byte stores GCC would emit. The offsets say otherwise: 0x04/0x05/0x06,
  0x0C/0x0D/0x0E, 0x14/0x15/0x16 and 0x1C/0x1D/0x1E are the canonical libgpu POLY_G4 vertex
  colour triples (rgb0..rgb3, i.e. the four setRGB0..setRGB3 field groups), and the file's own
  COMPLETED-C sibling func_80072BC4 (src/text1b.c:5822) is already written in exactly that
  field order. Written the natural way � each inner branch assigning its OWN complete rgb0 and
  rgb1 triple (whose red component is 0xFC on both branches), then the two unconditional
  rgb2/rgb3 triples � the function is byte-exact.
- mechanism: with the arms carrying full triples, GCC's jump2 cross-jump tail-merges the two
  arms' common tail (`sb v1,4 / sb v1,0xC / sb v0,0xE`) at the join label, which is precisely
  how target's merge block begins; the previously-diagnosed sched2 deferral of the `$v1` stores
  never arises because those stores are no longer emitted in the merge block by the source.
  The s2/s3 "two-attractor lock" was an artefact of the blob model: BOTH attractors lift the
  red components out of the colour assignments into a shared tail (`int fc_const`), which is
  the artificial spelling, not the natural one.
- probe: tmp/grind/func_80072CD4/s5/v_rgbtriple_noholder.c applied to src/text1b.c,
  `sandbox func_80072CD4 --disable all`.
- result: **score 0, build_insns 79 == target_insns 79, rules_dropped 0** (artifact
  tmp/grind/func_80072CD4/s5/sandbox_rgbtriple_noholder.json). The body contains NO local, no
  holder variable, no volatile, no asm, no barrier, no dead store, no annotation � only live
  field writes. The holder-retaining variant (tmp/grind/func_80072CD4/s5/v_rgbtriple.c, keeps
  the sibling's `int fc_const`) also measures 0.
- verdict: CONFIRMED (bytes measured this session, twice, reproducibly).

## [s5b] OPEN CLASSIFICATION QUESTION (the reason this session returns ruling-request, not candidate-ready)
- statement: the standing judge constraint of 2026-07-24 16:38 forbids respelling "the @4/@0xC
  common-tail stores (or any unconditional common-tail statement) as a duplicated-into-arms
  store-schedule lever". The byte-matching body above does place `= 0xFC` writes to 0x04 and
  0x0C inside both arms, so it is textually within reach of that ban � but it is not a lever
  bolted onto a hoisted base: there is no construct in the body at all, the writes are live on
  their own paths, and the alternative (hoisting only the red components behind an `int
  fc_const` holder) is the artificial spelling that the ban's own base form uses.
- probe: a ruling. In-repo precedent offered: func_80072BC4 is COMPLETED-C (absent from
  engine/queue.json) and carries an identical, hoistable-but-not-hoisted cross-arm duplicate
  store `*(u8 *)((s32)(arg1) + 0x1D) = 0xC3;` at src/text1b.c:5840 and src/text1b.c:5843.
- result: pending.
- verdict: OPEN � this is the whole frontier now. If the ruling says the ban does not reach a
  construct-free body, the function closes at 0 from memory/grind/func_80072CD4/candidate.c.
  If it says the ban does reach it, fall back to fallback_floor4.c (clean floor 4) and the
  two-attractor exhaustion analysis of s1-s5 stands unchanged.

## [s6] RESOLUTION of the s5b open classification question
- statement: the 2026-07-24 16:38 duplicated-into-arms constraint reaches the construct-free
  POLY_G4-triple body.
- probe: the filed ruling request (docs/grind/decisions.md:8390) was adjudicated.
- result: **PASS — the constraint does NOT reach it** (docs/grind/decisions.md:8448, on main as
  6e80ffdf). The ban is a ban on a LEVER (the `int fc_const` holder + merge-order-driven mid-arm
  injection of rejected/dup4_0xc_into_arms.c), not on the shape of an arm that writes its own
  complete colour triple. Declining to hoist a common LIVE store is ordinary C.
- verdict: KILLED (the hypothesis that the constraint blocks this body is false).

## [s6] Re-verification of the byte match on the current chassis
- statement: the s5b sandbox-0 measurement still holds on the chassis this session was dispatched
  against (the brief reported "measurement unavailable", so it could not be taken on trust).
- probe: apply candidate.c to src/text1b.c, `sandbox func_80072CD4 --disable all`.
- result: score 0, build_insns 79 == target_insns 79, rules_dropped 0
  (tmp/grind/func_80072CD4/s6/sandbox_s6_reverify.json).
- verdict: CONFIRMED.

## [s6] FRONTIER RESET
The frontier is not a search frontier any more — there is nothing left to search. The function
has a measured, ruled-on, construct-free pure-C byte match sitting in src/text1b.c. The only
remaining step is acceptance: layer-1 cheat-reviewer on the diff, then the Judge, then the
operator's oracle build + `queue done`. If (and only if) the Judge overturns the 2026-08-20 05:46
PASS, the fallback is memory/grind/func_80072CD4/fallback_floor4.c (clean floor 4, unchanged) and
the s1-s5 exhaustion analysis stands as written.


## [s5-forensics 2026-08-20] Which pass defers `sb v1,4 / sb v1,0xC` to the merge-block tail?
- statement: the residual-4 store order is set by **sched2** (schedule_insns pass 2), not by
  jump2's cross-jump splice and not by sched1, and it follows deterministically from the fact that
  the two stores have ZERO dependence predecessors inside the merge block.
- mechanism: sched.c `schedule_block` schedules BOTTOM-UP (the PICK trace is the exact reverse of
  the emitted order). Producer-less stores are ready in the first bottom-up round; among the
  equal-priority (pri=1) group `schedule_select` (sched.c:2660-2745) picks the one with the largest
  `potential_hazard`, which memory insns (unit=0) always win over unit=-1 insns. Picked first
  bottom-up == emitted last. Meanwhile every li->sb pair is dragged upward by `adjust_priority`'s
  birth boost (pri 0x7F000001) as soon as its consumer is scheduled.
- probe: instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_SCHED_DEBUG=1 over src/text1b.c carrying
  the cross-block body; SCHEDDBG FUNC/block/node/dep/PICK/SELBEST traces split into
  tmp/grind/func_80072CD4/s5/scheddbg_pass{1,2}.txt; plus the .flow/.sched RTL dumps
  (tmp/grind/func_80072CD4/dumps/, extracted to s5/f.flow, s5/f.sched).
- result: pass1 block=4 (the merge block) has n_ready=1 and picks 121,119,117,...,88,85 - i.e.
  sched1 leaves the merge block in SOURCE order (@4,@0xC,@0xE first), target-like. pass2 block=4
  lists NO dependence predecessors for insns 85 (@4), 88 (@0xC), 91 (@0xE), 114 (@0x16) and picks
  them at clock 3-6 (SELBEST 114, 91, 88, 85) -> they are emitted at the block TAIL. Disassembly
  agrees exactly.
- verdict: CONFIRMED (pass attribution settled at dump level; prior sessions attributed the order
  to jump2 and to "sched2 launch pairing" without a trace).

## [s5-forensics] Can ANY source that writes @4/@0xC once (in the merge block) reach target's order?
- statement: no. Target's merge head `sb v1,4 / sb v1,0xC / sb v0,0xE` is unreachable from any
  source that writes those stores in the merge block, in any statement order.
- mechanism: two facts compose. (1) sched2 sinks producer-less stores (above), and it rebuilds the
  order from a dependence graph that is identical for every permutation of the source statements -
  which is why s4b's 15.8k-iteration directed PERM_LINESWAP over exactly those stores found nothing
  below base. (2) toplev.c pass order: sched2 at :3117, `jump_optimize (insns, 1, 1, 0)` (the
  cross_jump run) at :3142, dbr at :3167 - cross-jump runs AFTER sched2, so a common tail spliced
  at the join label is NEVER re-scheduled and keeps the head slot.
- probe: read toplev.c:3105-3167; internal control in this function's own floor-4 build
  (tmp/grind/func_80072CD4/s5/base.dis): `sb v0,0xE` - the ONE store this body writes per-arm - sits
  at the merge head (0x876c) because jump2 cross-jumped it post-sched2, while `sb v1,4`/`sb v1,0xC`,
  written in the merge block, sink to 0x8794/0x8798. Same block, same passes, opposite placement;
  the only difference is which block the source wrote them in.
- verdict: CONFIRMED. Corollary: the only source shape that can produce target's merge head is one
  that writes @4/@0xC (and @0xE) inside BOTH arms - i.e. the shape the layer-1 reviewer banned. This
  is a mechanism argument about what the ORIGINAL source must have contained, not a new lever.

## [s5-forensics] Why the cross-block chassis (13/78) cannot be repaired
- statement: sched1's hoist of the arms' `li var_v0,0x32/0x46` to the arm top is forced by the same
  bottom-up rule and cannot be undone by statement order.
- mechanism: scheddbg_pass1.txt block=2: insn 57 `(set (reg/v:QI 75) (const_int 50))` has unit=-1,
  icost=0, pri=1 and NO in-block consumer (its consumer, insn 91, is in the merge block), so it
  loses SELBEST to the unit=0 stores at clock 2/4/6 and is picked last (clock 8) => emitted first
  => arm top. Identical trace for insn 80 in the ELSE arm (block=3).
- result: hoist makes both arm tails `sb v0,0xD` identical, so jump2 cross-jumps THAT store out of
  the arms (78 insns, one short of target's 79) and RA rotates (fc_const -> $a0, var_v0 -> $v1).
  Measured this session at 13/78, chassis-current.
- verdict: KILLED (attractor b is closed by a compiler rule, not by a search gap). Its only escape
  would be giving the arm `li` an in-block consumer - which means storing @0xE in the arm, i.e.
  moving to the per-arm chassis.

## [s5-forensics-2 2026-08-20] Is the per-arm placement a merge-order lever, as layer-1 characterised it?
- statement: the layer-1 FAIL's factual premise — that `@4 = 0xFC` / `@0xC = 0xFC` sit in both arms
  "solely to steer jump2's cross-jump merge point" — is false; the C cannot steer that merge order
  at all.
- mechanism: the arms' emitted tail order (4, C, E) is produced by sched2's bottom-up sinking of
  producer-less stores, applied separately to each arm, from a SOURCE order that is canonical
  ascending POLY_G4 field order (4,5,6,C,D,E). jump2 runs after sched2 (toplev.c: sched2 :3117,
  cross_jump :3142) and splices the already-scheduled common tail at a newly created join label.
- probe: `pwsh tools/grinder/dump.ps1 func_80072CD4` with candidate.c applied; per-function RTL in
  tmp/grind/func_80072CD4/s5f2/func_80072CD4.{sched2,jump2}.rtl.
- result: sched2 THEN arm ends 45(@5) 50(@6) 60(@0xD) | 40(@4) 55(@0xC) 65(@0xE); ELSE arm ends
  80(@5) 85(@6) 95(@0xD) | 75(@4) 90(@0xC) 100(@0xE). jump2 deletes 40/55/65, inserts code_label
  223 (label 872) before 75/90/100 → merge head = target's merge head.
- verdict: KILLED (the "lever" characterisation is disproved at dump level; the source order and
  the emitted order are different orders, and only the compiler produces the second).

## [s5-forensics-2] Does a cross-arm duplicate store require jump2 to merge it to be house style?
- statement: no — the accepted, COMPLETED-C sibling carries one that jump2 declines to merge.
- probe: same dumps, func_80072BC4 slice (tmp/grind/func_80072CD4/s5f2/func_80072BC4.jump2.rtl).
- result: `@0x1D = 0xC3` written at src/text1b.c:5840 and :5843 survives as TWO insns (75, 88) in
  jump2; the function is zero-rule, byte-matched and absent from engine/queue.json.
- verdict: CONFIRMED (the spelling is accepted in-repo on its own terms).

## [s5-forensics-2] FRONTIER
Empty as a search frontier. The function has a chassis-current byte match (score 0, 79 == 79,
rules_dropped 0) landed at src/text1b.c:5865, a Judge PASS on its classification
(docs/grind/decisions.md:8456, commit a8d7ee5f), a construct-free self-vet, and now a dump-level
disproof of the only factual objection raised against it. What remains is acceptance: layer-1 on
the diff, the Judge, then the operator's oracle build + `queue done`. If layer-1 or the Judge
overturns the 06:09 ruling, the fallback is memory/grind/func_80072CD4/fallback_floor4.c (clean
floor 4, unchanged) and the s1-s5 exhaustion analysis stands as written.
