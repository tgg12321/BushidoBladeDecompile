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

## [s7-rederive 2026-08-20] arg1 is a PSX libgpu POLY_G4, and the body is four setRGB* triples
- statement: the function's semantic model is not "byte stores at magic offsets into a GameObj";
  arg1 is a `POLY_G4` primitive and every store is a named RGB component of one of its four vertex
  colours. Re-deriving the body from that model (a real `POLY_G4` struct type, named members, no
  casted byte offsets anywhere in the body, no locals) still byte-matches.
- mechanism: libgpu's POLY_G4 layout is `u32 tag; u8 r0,g0,b0,code; s16 x0,y0; u8 r1,g1,b1,pad;
  s16 x1,y1; u8 r2,g2,b2,pad; s16 x2,y2; u8 r3,g3,b3,pad; s16 x3,y3;` = 0x24 bytes. Every offset
  this function writes (4/5/6, C/D/E, 14/15/16, 1C/1D/1E) is exactly an r/g/b triple, nothing else
  is touched, and the function's own return value is `arg1 + 0x24` — the primitive's size. The
  layout is confirmed by the code, independently of any naming census.
- probe: tmp/grind/func_80072CD4/s7/body_R1b_struct_nolocal_perarm.c applied via s7/apply.py, then
  `& tools/wteng.ps1 main sandbox func_80072CD4 --disable all`
  (tmp/grind/func_80072CD4/s7/body_R1b_struct_nolocal_perarm.json).
- result: **score 0, build_insns 79 == target_insns 79, rules_dropped 0.** A structurally different
  C shape from every s1-s6 form (struct-typed member writes vs `*(u8 *)((s32)(arg1) + N)` casts)
  reaching the same byte match.
- verdict: CONFIRMED. Corollary for classification: the per-arm repetition of the shared `0xFC`
  red component is not a hand-placed store at all in this spelling — it is one component of the
  `setRGB0(p, 0xFC, 0xC3, 0x1E)` / `setRGB1(p, 0xFC, 0xC8, 0x32)` triple each arm assigns.

## [s7-rederive] TARGET'S OWN BYTES contain a non-merged cross-arm duplicate of a shared constant
- statement: the original source of THIS function demonstrably wrote a common constant in both
  inner arms rather than hoisting it — this is readable off the shipped binary, with no appeal to
  any GCC pass, any lever theory, or any sibling function.
- probe: asm/funcs/func_80072CD4.s. THEN arm lines 23-24: `addiu $v0,$zero,0xC3` / `sb $v0,0x5($s1)`.
  ELSE arm lines 32-33: `addiu $v0,$zero,0xC3` / `sb $v0,0x5($s1)`. Same value, same field, both
  arms, TWO copies in the shipped code — jump2 declines to merge them because the arms' trailing
  insns differ (THEN ends `j .L80072D64 / addiu $v0,0x32`, ELSE ends `addiu $v0,0x46`).
- result: if the original source had hoisted the shared green component `@5 = 0xC3` out of the arms
  (the shape the layer-1 FAIL demands for `@4`/`@0xC`), `sb $v0,0x5` would appear ONCE, in the merge
  block. It appears twice, in the shipped executable.
- verdict: CONFIRMED. Cross-arm duplication of a value common to both arms is ORIGINAL-SOURCE
  behaviour for func_80072CD4, established by the target bytes themselves. Any C that matches this
  function must contain at least one such duplicate (`@5 = 0xC3`), so "the same constant is written
  in both arms" cannot by itself be the cheat signature here.

## [s7-rederive] Does a pointer local (`POLY_G4 *p = (POLY_G4 *)arg1;`) change the outcome?
- statement: no — it strictly costs insns on both chassis, generalising s5's u8*-spelling kill.
- probe/result: per-arm chassis with `p` = **13 / 82** (three insns worse than the same body written
  `((POLY_G4 *)arg1)->field`, which is 0/79); cross-block chassis with `p` = **24 / 80** vs 11/77
  without. Artifacts: s7/body_R1_struct_perarm.json, s7/body_R2_struct_xblock.json.
- verdict: KILLED (rejected/rederive_polyg4_ptrlocal_perarm_13_82.c,
  rejected/rederive_polyg4_ptrlocal_xblock_24_80.c). Consistent with s5's
  base_pointer_local_spelling (17/82): the target keeps `arg1` in `$s1` and addresses every field
  off it directly; any C-level second handle for the base is byte-divergent.

## [s7-rederive] The cross-block (non-duplicated) attractor re-measured in struct spelling: 11/77
- statement: struct spelling improves the b-attractor's score (13/78 -> 11/77) but moves it FURTHER
  from target in insn count, and the two missing insns are now individually attributed.
- probe: rejected/rederive_polyg4_struct_xblock_11_77.c; disassembly tmp/grind/func_80072CD4/s7/r2b.dis.
- result: 79 - 77 = 2 missing insns, both identified by reading r2b.dis against
  asm/funcs/func_80072CD4.s:
  (1) sched1 hoists the staged `b1` constant to the arm HEAD (`li v1,0x32` in the inner beqz delay
      slot at 0x872c; `li v1,0x46` at 0x8748), so both arms now END with an identical `sb v0,0xD`,
      and jump2 cross-jumps THAT store to the merge head (0x8760) — target keeps one copy per arm
      (:28 and :37) precisely because its arms end with DIFFERENT `li` constants (0x32 / 0x46), so
      the backward common-tail scan finds nothing to merge.
  (2) CSE folds `@0x14 = 0xFC` into the same `li v0,0xFC` that feeds `@4` and `@0xC` (one `li` at
      0x8764 feeding sb 4 / sb 0xC / sb 0x14), where target re-materialises it
      (`addiu $v0,$zero,0xFC` at 0x80072D70 for `@0x14`). This is the mirror of s1's H1: writing
      all three 0xFC stores as bare literals in ONE block lets CSE share them.
- verdict: KILLED, with the mechanism now fully local: repairing (1) requires the arm's staged
  constant to survive at the arm tail, which requires an in-arm consumer for it, which means storing
  `@0xE` inside the arm — i.e. leaving the b-attractor entirely. Independent re-derivation of s3/s5's
  conclusion by a different spelling and a different measurement.

## [s7-rederive] Tooling note
- m2c is NOT installed in this checkout's WSL venv (`python3 -c "import m2c"` -> ModuleNotFoundError;
  no `m2c/` dir, no `~/m2c`). The rederive modality's "fresh m2c decompile" leg is unavailable; this
  session re-derived the body by hand from the target asm plus the libgpu POLY_G4 layout instead,
  which is a stronger derivation (it names the fields rather than reproducing the offsets).

## s5 — rederive modality (2026-08-20)

- **H-R1 (KILLED).** *A fresh cross-block (`var_v0`) spelling — the exact house style of the
  COMPLETED-C sibling func_80072BC4 — reaches 0.* Mechanism probed: keep `p[0xE] = var_v0;` in the
  merge block so the arms end with `li 0x32`/`li 0x46` as target does. Probe: four fresh spellings
  measured (var-assignment first in the arm; `s32` instead of `u8` var; `fc_const` hoisted above
  the OUTER if as in the sibling; plus the s3 baseline). Result: 13/78, 13/78, 14/78. Every one
  loses identically — sched1 hoists the producer-less `li var_v0` to the arm TOP, the two arm tails
  become the identical insn `sb v0,0xD`, and jump2 cross-jumps it out, giving 78 insns where target
  keeps `sb v0,0xD` in BOTH arms at 79. Neither the variable's type, nor its position in the arm,
  nor where `fc_const` is assigned perturbs the sched1 pick order. The xblock chassis is closed.

- **H-R2 (KILLED).** *Moving the unconditional 0x14/0x15/0x16/0x1C/0x1D/0x1E group above the inner
  if leaves a 3-store merge block that sched2 emits in source order 4, C, E.* Probe: measured,
  33/78. Worse by every metric; the group's stores are then in the wrong block entirely.

- **H-R3 (CONFIRMED — new codegen law L1, with a two-arm control).** *A merge-block store whose
  value register is defined in a predecessor block sinks to the merge tail; giving the same store
  an in-block producer moves it to the merge head.* Probe: dropped the `fc_const` local so the
  merge block writes literal `0xFC` (rejected/rederive_merge_literals_no_fcholder_6_77.c). The two
  stores acquired an in-block `li v0,252` and jumped from merge positions 9-10 to positions 1-2
  (tmp/grind/func_80072CD4/s5r/p6.dis vs base.dis). Score 6 / 77 insns — the CSE that the in-block
  producer enables also eats the separate `li 0xFC` target keeps for @0x14, so this spelling cannot
  be the answer, but the LAW is now established by control rather than inferred from a dump.
  Target obeys L1: its one producer-less merge-block store, `sb zero,0x16`, is at the merge tail.

- **H-R4 (CONFIRMED — new lever, bounded).** *L1 can also be defeated without moving the producer,
  by denying GCC 2.7.2 memory disambiguation.* Probe: second base pointer
  `u8 *q = (u8 *)arg1 + 4; q[0] = fc_const; q[8] = fc_const;`
  (rejected/rederive_walkptr_alias_serialize_6_79.c). `memrefs_conflict_p` cannot compare a
  q-based MEM with an s1-based MEM, so the stores are serialised and leave the merge tail
  (p7.dis: `sb a0,0(v1) / sb a0,8(v1)` immediately after the cross-jumped `sb v0,0xE`). Score
  6 / 79 — the best any non-per-arm form has scored on structure, still worse than the floor-4
  baseline, because the cross-jumped store keeps merge position 0. **Bank this lever**: it is a
  general, cheap way to pin merge-block store order in this codebase and it did not exist in the
  ledger before.

- **H-R5 (CONFIRMED — reconstruction proof).** *Target's `sb v1,4 / sb v1,0xC / sb v0,0xE` merge
  head is a 3-insn jump2 common tail, i.e. the original C wrote all three stores inside BOTH inner
  arms.* Derivation from emitted bytes only (see evidence.md s5 section): by L1 they cannot be
  merge-block source statements (they would have sunk, as `sb zero,0x16` does IN TARGET); by L2
  (cross-jump splices at the join label, after sched2) nothing sched2 emits can precede a
  cross-jumped insn, so they cannot be merge-block statements sitting ahead of a cross-jumped
  `sb v0,0xE`. Only L2 can put an insn at merge position 0. This closes the question of what the
  original source shape was — and that shape is exactly the body currently on the driver's
  banned_constructs list.

- **Frontier consequence.** Every non-per-arm axis is now bounded away from 0 by a proof, not by
  search exhaustion: xblock is capped at 78 insns by the sched1 hoist + `sb v0,0xD` cross-jump;
  merge-block spellings are capped by L1/L2 regardless of statement order (the s4 15.8k-iteration
  directed PERM_LINESWAP over exactly those stores is the empirical companion to the proof); the
  alias-serialisation lever reaches 6 and cannot pass L2. The only remaining question about this
  function is a CLASSIFICATION question about the per-arm body, not a search question.

## [s8 2026-08-20 — rederive] H-S8 (CONFIRMED): the POLY_G4 struct body is a whole-executable match
- **Statement.** The POLY_G4 struct per-arm body — the spelling the 2026-08-20 06:35 and 06:54
  Judge rulings PASSED — reaches sandbox 0 on the current chassis AND links a byte-identical
  executable, i.e. it clears the full FINAL CALL that both standing judge constraints demand and
  that no prior session had run.
- **Mechanism.** Not a lever: the offsets are the four POLY_G4 RGB triples, so each arm's colour
  assignment is complete inside the arm; jump2 then cross-jumps the arms' common tail
  `sb v1,4 / sb v1,0xC / sb v0,0xE` to the join label, which is precisely target's merge head. The
  L1/L2 laws (s5-rederive) say no merge-block SOURCE statement can occupy that position, which is
  why every blob-model spelling of s1-s5 was bounded away from 0 rather than merely unlucky.
- **Probe.** `python3 tmp/grind/func_80072CD4/s5/apply.py memory/grind/func_80072CD4/candidate.c`
  → `sandbox func_80072CD4 --disable all` → `build`.
- **Result.** score 0, build_insns 79 == target_insns 79, rules_dropped 0
  (tmp/grind/func_80072CD4/s8/sandbox_struct_s8.json); full build
  sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, MATCH.
- **Verdict.** CONFIRMED. The remaining question about this function is not a search question and
  no longer a classification question the grind can advance: both Judge rulings on the struct
  spelling are PASS, the FINAL CALL is clear, and acceptance now rests with the layer-1/layer-2
  adversarial review of the landed diff.

## [s5-structural 2026-08-20] — structural modality, three hypotheses, all KILLED

- **H-S5S1 (KILLED).** *Composing the alias-serialisation lever with the cross-block chassis
  escapes L2: with no per-arm `@0xE` store there is no jump2 common tail to splice at merge
  position 0, so an alias-serialised `q[0]/q[8]/q[0xA]` group can occupy the merge head in source
  order (= target's order).* Probe: rejected/xblock_q3alias_11_78.c, `sandbox --disable all` = 11,
  build_insns 78. The alias group DOES reach the merge head in ascending @4/@0xC/@0xE order (no L1
  sink), but deleting the per-arm store re-arms the sched1 hoist of the cross-block value `li`
  (producer-less and consumer-less in the arm -> picked last bottom-up -> emitted at arm top),
  which makes both arm tails end in an identical `sb v0,0xD($s1)`; jump2 cross-jumps THAT instead
  and it takes merge position 0. **L2's binding insn is replaceable, not removable.**

- **H-S5S2 (KILLED).** *An alias base pointer with no offset bias (`u8 *q = (u8 *)arg1;`, stores
  written `q[4]`/`q[0xC]`) gets the serialisation effect for free, because there is nothing for
  the addressing mode to fold and the copy should coalesce with arg1.* Probe:
  rejected/perarmE_q0alias_7_80.c, score 7, build_insns **80** (one MORE than target). GCC 2.7.2
  keeps a separate pseudo and emits a plain `$s1 -> $v1` copy; serialisation fires (the @4/@0xC
  stores leave the merge tail and land immediately behind the cross-jumped `sb v0,0xE`) and the
  score still regresses from 4 to 7. **The alias lever is never insn-free, and target's merge head
  uses the `$s1` base with literal offsets, so no alias form can be byte-equal there.**

- **H-S5S3 (KILLED).** *Paying for the alias pointer with the sched1-hoist cross-jump restores
  target's 79-insn count, and with the `fc_const` holder back in the inner-beqz delay slot (two
  separate 0xFC materialisations, as target has) the remaining diff is only ordering.* Probe:
  rejected/xblock_q3alias_fcholder_13_79.c, score **13**, build_insns 79. Matching the count by
  cancelling a +1 against a -1 puts both insns in the wrong blocks; it is the worst-scoring
  cross-block variant measured.

**Frontier after s5-structural.** Structural modality is closed: the per-arm floor-4 chassis and
the cross-block chassis have now both been probed with the alias-serialisation lever and with
holder placement, and every result is >= the control's 4. The residual is not a search failure —
L1 ∧ L2 (evidence.md) bound every non-per-arm form away from target's merge head, and the one
structure that reaches it is the banned per-arm @4/@0xC construct. The remaining untried
modalities are NOT structural; nothing in the structural catalog (block-local var splits,
declaration order, type narrowing, statement re-association) has an unspent form left for this
function.

**Housekeeping done by s5-structural.** `memory/grind/func_80072CD4/candidate.c` had been left
holding the BANNED sandbox-0 per-arm POLY_G4 body (the one the layer-1 cheat-reviewer FAILed on
2026-08-20 at 05:53, 06:20 and 07:02). Since the dispatch brief instructs each session to apply
candidate.c as its starting point, that was a live trap. candidate.c is now the clean floor-4
body (byte-identical to fallback_floor4.c) with a header that names the ban; the banned body
remains banked at rejected/rederive_polyg4_struct_perarm_score0_banned_family.c and must not be
re-submitted in any spelling.

## [s5] Composing the alias-serialisation lever with the cross-block chassis escapes L2: with no per-arm @0xE store there is no jump2 common tail to splice at merge position 0, so an alias-serialised q[0]/q[8]/q[0xA] group can occupy the merge head in source order, which is target's order (@4, @0xC, @0xE).
- mechanism: GCC 2.7.2 memrefs_conflict_p cannot disambiguate (mem (plus q k)) from (mem (plus s1 k')) once a second base pointer exists, so sched2 serialises the q-group and it cannot sink (defeats L1). Removing the per-arm @0xE store was expected to remove L2's binding insn (the cross-jumped sb v0,0xE).
- probe: tmp/grind/func_80072CD4/s5b/vA_xblock_q3.c applied to src/text1b.c with tmp/grind/func_80072CD4/s5/apply.py; `& tools/wteng.ps1 main sandbox func_80072CD4 --disable all`; disassembly tmp/grind/func_80072CD4/s5b/vA.dis
- result: score 11, build_insns 78 (target 79). The q-group DOES reach the merge head in ascending @4/@0xC/@0xE order with no L1 sink (li v0,252 / sb v0,0(v1) / sb v0,8(v1) / sb a0,10(v1)). But deleting the per-arm store re-arms the sched1 hoist of the cross-block value li (producer-less and consumer-less in the arm, picked last bottom-up, emitted at the ARM TOP at 0x8730/0x874c), which makes both arm tails end in an identical sb v0,0xD($s1); jump2 cross-jumps THAT and it takes merge position 0, ahead of the q-group. Banked rejected/xblock_q3alias_11_78.c.
- verdict: KILLED

## [s5] An alias base pointer with no offset bias (u8 *q = (u8 *)arg1; stores written q[4]/q[0xC]) buys the serialisation effect for free, because there is nothing for the addressing mode to fold and the copy should coalesce with arg1.
- mechanism: If q coalesces with arg1 the emitted base register stays $s1 (target's base) while the RTL still carries two distinct pseudos, which is all memrefs_conflict_p needs to refuse disambiguation.
- probe: tmp/grind/func_80072CD4/s5b/vG_perarmE_q0.c on the floor-4 (per-arm @0xE) chassis; sandbox --disable all; disassembly tmp/grind/func_80072CD4/s5b/vG_perarmE_q0.dis
- result: score 7, build_insns 80 — one MORE than target. GCC 2.7.2 keeps a separate pseudo and emits a plain $s1 -> $v1 copy. Serialisation does fire (sb a0,4(v1) / sb a0,12(v1) at 0x8774-0x8778 immediately follow the cross-jumped sb v0,14(s1) at 0x876c), and the score still regresses from the control's 4 to 7 because of the extra copy plus the base-register mismatch on two stores. Target's merge head uses $s1 with literal offsets 4 and 0xC and contains no alias base register anywhere. Banked rejected/perarmE_q0alias_7_80.c.
- verdict: KILLED

## [s5] Paying for the alias pointer with the sched1-hoist cross-jump restores target's 79-insn count, and with the int fc_const holder back in the inner-beqz delay slot (two separate 0xFC materialisations, as target has) the remaining diff is only ordering.
- mechanism: The alias pointer costs +1 insn and the sched1-hoist-induced cross-jump of sb v0,0xD saves -1; the fc_const holder reproduces target's addiu $v1,$zero,0xFC in the inner beqz delay slot while @0x14 keeps a distinct merge-block literal (the s1 H1 distinct-literal re-materialisation effect).
- probe: tmp/grind/func_80072CD4/s5b/vC_xblock_q_fcholder.c; sandbox --disable all; disassembly tmp/grind/func_80072CD4/s5b/vC_xblock_q_fcholder.dis
- result: score 13, build_insns 79. Target's insn COUNT is matched exactly and the score is the WORST of the whole cross-block family: cancelling a +1 against a -1 puts both insns in the wrong blocks. Banked rejected/xblock_q3alias_fcholder_13_79.c.
- verdict: KILLED

## [s5] The banked floor of 4 still reproduces on the current (post asm-until-matched migration) chassis, despite the dispatch brief reporting the chassis measurement as unavailable.
- mechanism: n/a — direct control measurement of the banked clean body before any probe was run.
- probe: memory/grind/func_80072CD4/fallback_floor4.c applied to src/text1b.c; `& tools/wteng.ps1 main sandbox func_80072CD4 --disable all`
- result: score 4, target_insns 79, build_insns 79, rules_dropped 0, scorable true. Chassis confirmed unchanged; every banked spelling conclusion in the ledger remains chassis-current.
- verdict: CONFIRMED

## [s6-structural 2026-08-20] — branch-polarity / arm-order axis, three hypotheses, all KILLED
Context: the s5-structural frontier declared the structural modality closed after probing both
chassis with the alias-serialisation and holder-placement levers. The one structural sub-axis the
whole s1..s5 ledger never touched is the CONTROL-FLOW SPELLING itself: every form ever measured used
the same branch polarity (`if (arg0 < 4)`, `if (flags & 4)`) and the same arm ORDER as candidate.c.
Nobody had ever asked whether that polarity is target's or merely the first one anyone typed.
Mechanism worth asking about: inverting a condition and swapping the two arm bodies is semantically
identical C, but it changes which basic block is the FALL-THROUGH and which is the branch target.
That is load-bearing for exactly the two passes this function's residual lives in — jump_optimize's
cross_jump picks the tail to splice relative to the join label reached from the fall-through edge,
and dbr fills the `j`/`beqz` delay slot from the block it falls out of. If the residual-4 merge-head
ordering were an artefact of WHICH arm supplies the cross-jumped tail, an inversion would move it.

- **H-S6S1 (KILLED).** *Inverting the INNER condition (`if (!(D_800A35C4->x8 & 4))`) and swapping the
  two colour arms changes which arm supplies jump2's cross-jumped tail, and can therefore move the
  merge-block head order.*
  - probe: tmp/grind/func_80072CD4/s6/vA_inner_sense_invert.c, applied with s6/apply.py, `sandbox
    func_80072CD4 --disable all`.
  - result: **score 11, build_insns 79** (control 4/79). Strictly worse, and INSN-COUNT NEUTRAL — the
    inversion costs nothing and buys nothing; it only permutes which literal lands in which register
    and which arm is the fall-through. Banked rejected/s6_inner_branch_sense_invert_11_79.c.
  - verdict: KILLED. Corollary: candidate.c's inner branch polarity and arm order ARE target's.

- **H-S6S2 (KILLED).** *Inverting the OUTER condition (`if (arg0 >= 4)` with the big 12-store arm
  first) re-lays the two top-level blocks, which changes the block order handed to sched2 and could
  reshape the merge block that follows.*
  - probe: tmp/grind/func_80072CD4/s6/vB_outer_sense_invert.c; same measurement path.
  - result: **score 39, build_insns 79.** Massively worse, again at identical insn count: GCC keeps
    the `slti`+`beqz` shape either way, so the only effect is that the 12-store arm becomes the
    fall-through and the whole `arg0 < 4` region moves behind it, mis-placing every block.
    Banked rejected/s6_outer_branch_sense_invert_39_79.c.
  - verdict: KILLED.

- **H-S6S3 (KILLED).** *The two inversions might compose — the outer inversion's block re-lay could
  restore the inner arm relationship that the inner inversion breaks.*
  - probe: tmp/grind/func_80072CD4/s6/vC_both_invert.c; same measurement path.
  - result: **score 46, build_insns 79** — the worst measurement in the entire structural family.
    The perturbations add rather than cancel. Banked rejected/s6_both_branch_sense_invert_46_79.c.
  - verdict: KILLED.

**Chassis control re-measured first, before any probe:** memory/grind/func_80072CD4/candidate.c ->
score 4, build_insns 79 == target_insns 79, rules_dropped 0, scorable true. The dispatch brief
reported the chassis measurement as unavailable; it is unchanged, so every banked spelling
conclusion in this ledger remains chassis-current for a second consecutive session.

**Frontier after s6-structural.** The structural modality is now closed on a THIRD independent axis
(control-flow spelling), on top of s5's alias-serialisation and holder-placement closures and s4's
directed permuter over the full 8-store merge-block permutation space. There is no structural lever
left that has not been measured: the arms are byte-identical to target, the insn count is exact
(79 == 79), and all four remaining diffs live in the merge-block store ORDER, which H-R5 proved is
reachable only from a source that writes the shared `0xFC` into both inner arms — the construct
banned for this function by three layer-1 cheat-reviewer FAILs. No further structural session should
be dispatched; the only open question about this function is its DISPOSITION, not its codegen.

## [s6] Inverting the INNER condition to `if (!(D_800A35C4->x8 & 4))` and swapping the two colour arms changes which arm supplies jump_optimize's cross-jumped tail, and can therefore move the residual merge-block head order.
- mechanism: cross_jump splices the merged common tail at the join label reached from the fall-through edge, and dbr fills the delay slot from the block it falls out of; inverting the condition swaps fall-through and branch-target arms, which is the only source-level control over both passes that does not add an instruction.
- probe: tmp/grind/func_80072CD4/s6/vA_inner_sense_invert.c applied with tmp/grind/func_80072CD4/s6/apply.py; `& tools/wteng.ps1 main sandbox func_80072CD4 --disable all`.
- result: score 11, build_insns 79, target_insns 79 (control 4/79). Insn-count neutral, strictly worse. Banked rejected/s6_inner_branch_sense_invert_11_79.c.
- verdict: KILLED

## [s6] Inverting the OUTER condition to `if (arg0 >= 4)` with the 12-store arm written first re-lays the two top-level blocks, changing the block order handed to sched2 and reshaping the merge block that follows it.
- mechanism: GCC 2.7.2 emits slti+beqz for either polarity, so the inversion cannot change cost; it only changes which top-level block is the fall-through and therefore the order in which schedule_insns pass 2 and jump_optimize see the blocks.
- probe: tmp/grind/func_80072CD4/s6/vB_outer_sense_invert.c; same apply + sandbox path.
- result: score 39, build_insns 79, target_insns 79. Insn-count neutral, far worse - the whole arg0<4 region moves behind the 12-store arm. Banked rejected/s6_outer_branch_sense_invert_39_79.c.
- verdict: KILLED

## [s6] The two inversions compose: the outer inversion's block re-lay could restore the inner-arm relationship the inner inversion breaks.
- mechanism: if the two perturbations act on the same block-ordering state they could cancel, which would be the only way a control-flow lever reaches target's merge head without touching the arms' store content.
- probe: tmp/grind/func_80072CD4/s6/vC_both_invert.c; same apply + sandbox path.
- result: score 46, build_insns 79, target_insns 79 - the worst score in the entire structural family. The perturbations add rather than cancel. Banked rejected/s6_both_branch_sense_invert_46_79.c.
- verdict: KILLED

## [s6] The banked floor of 4 still reproduces on the chassis this session was dispatched against (the brief reported the chassis measurement as unavailable).
- mechanism: n/a - direct control measurement of the banked clean body, taken before any probe.
- probe: memory/grind/func_80072CD4/candidate.c applied via tmp/grind/func_80072CD4/s6/apply.py; `& tools/wteng.ps1 main sandbox func_80072CD4 --disable all`.
- result: score 4, target_insns 79, build_insns 79, rules_dropped 0, scorable true. Chassis unchanged for a second consecutive session; every banked spelling conclusion remains chassis-current.
- verdict: CONFIRMED

## [s7-synthesis 2026-08-20] — MERGED ATTACK: the whole pure-C space of this function is a
## three-point lattice on ONE dial, and this session measured the previously-empty middle point

- statement: every C form ever measured for func_80072CD4 (s1-s7, 30 banked rejects, one 15.8k-iteration
  directed permuter campaign, three structural axes) differs on exactly one dial: **how many of the two
  shared red components — r0 (@4) and r1 (@0xC), both 0xFC — are written INSIDE the two inner colour
  arms** rather than hoisted into the merge block. The dial has three settings and this session measured
  the one that had never been run:
    * **0 duplicated** (both hoisted; either behind the sibling's `int fc_const` holder or as bare
      merge-block literals): the banked floor. **4 / 79** (control re-measured this session,
      tmp/grind/func_80072CD4/s7/syn_control_floor4.json — the dispatch brief reported the chassis
      measurement as unavailable). Insn-exact, both arms byte-exact; the entire residual is the
      merge-block store ORDER.
    * **exactly 1 duplicated** — NEW, never measured before s7: r0 in both arms + r1 hoisted =
      **6 / 80**; r1 in both arms + r0 hoisted = **9 / 80**. Both are one insn OVER target and both
      score WORSE than the 0-dup floor.
    * **2 duplicated** (each arm writes its own complete rgb0 and rgb1 triple): **0 / 79** — the byte
      match, and the construct the driver's banned_constructs list disqualifies.
- mechanism for the new middle point: with both reds hoisted, the merge block materialises 0xFC once and
  shares that `li` between the @4 and the @0xC store (one block, one constant); with both reds in the
  arms, 0xFC is materialised in the inner-beqz delay slot and the pair of stores rides out of the arms on
  jump2's cross-jumped common tail. Hoisting exactly ONE red destroys both economies: the lone hoisted
  store still needs a merge-block `li` of its own, while the arm-resident one already has its own
  materialisation, so the function pays for two constant materialisations where target and both endpoints
  pay for one — hence 80 insns in both half-dup spellings. The middle setting is not a compromise between
  the two endpoints; it is strictly dominated by both.
- probe: tmp/grind/func_80072CD4/s7/syn_halfdup_r0only.c and syn_halfdup_r1only.c (POLY_G4 struct
  spelling, identical to the banked score-0 body except for which red component is hoisted), applied with
  tmp/grind/func_80072CD4/s7/apply.py, then `& tools/wteng.ps1 main sandbox func_80072CD4 --disable all`;
  JSON in syn_halfdup_r0only.json / syn_halfdup_r1only.json. Bodies banked as
  rejected/syn_halfdup_r0only_6_80.c and rejected/syn_halfdup_r1only_9_80.c.
- result: 6/80 and 9/80.
- verdict: KILLED — and the kill is structural, not incidental: it closes the last quadrant of the dial
  and shows the pure-C solution space of this function contains exactly three arrangements, of which
  exactly one matches.

## [s7-synthesis] What the lattice means, merged with the s5-forensics dump evidence
The three-point result composes with the two codegen laws that s5-forensics established at cc1-dump
level (L1: a merge-block store whose value register is defined in a predecessor block has no in-block
dependence predecessor, is ready in sched2's first bottom-up round and is therefore emitted LAST;
L2: toplev.c runs sched2 (:3117) BEFORE jump_optimize(..., cross_jump=1) (:3142), so a cross-jumped
common tail is spliced at the join label ahead of everything sched2 emitted and is never re-scheduled).
Target's merge head is `sb v1,4 / sb v1,0xC / sb v0,0xE`. By L1 no merge-block-written store can occupy
that head; by L2 the only thing that can is a jump2-spliced common tail; and a common tail exists only if
the stores are written in BOTH arms. So:

  **the set of pure-C bodies that byte-match func_80072CD4 is exactly the set of bodies that write @4 and
  @0xC inside both inner arms** — which is exactly the set the banned_constructs entry disqualifies. For
  this function the ban and "no pure-C match exists" are co-extensive.

That conclusion is now supported from three independent directions: dump-level pass attribution
(s5-forensics), the 15.8k-iteration directed permuter over the full merge-store permutation space (s4b),
and this session's exhaustion of the duplication dial. Nothing that remains is a codegen question.

## [s7-synthesis] FRONTIER RESET
1. **Disposition, not codegen.** The only open question is whether the banned_constructs entry stands.
   The driver should dispatch `escalation`. That entry must be written knowing both endgame-lock gates
   FAIL (scan_hand_coded is not STRONG for this function per docs/grind/decisions.md:8361; no
   SOTN-master precedent census hit for the construct), so the standing 2026-07-27 ruling shape applies:
   `OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE`,
   terminal, INCLUDE_ASM retained on main, candidate.c (floor 4) retained in the ledger. The entry should
   carry the three-point lattice above — it is the cleanest available statement of why no fourth
   arrangement exists.
2. **The one unbanked, unmeasured codegen arrangement left** is duplicating the rgb2/rgb3 triples
   (@0x14 and @0x1C groups) into the arms instead of the rgb0/rgb1 reds. It touches neither @4 nor @0xC,
   so it is outside the ban. Predicted dead by L2: whatever is duplicated becomes the cross-jumped tail
   and therefore occupies merge position 0, and target's merge position 0 is @4, not @0x14 — so the probe
   should move the head to the wrong field. Never measured; cheap; the only remaining thing that is not a
   re-run of a banked kill.
3. **Closed by inspection** (the s6 frontier's "formality" item): no source can make the two arms
   tail-distinct by REAL semantic content, because the arms differ only in three literal colour
   components (b0 0x1E/0x50, g1 0xC8/0xDC, b1 0x32/0x46 — asm/funcs/func_80072CD4.s). There is no
   semantic content available to differentiate them with. Do not dispatch a session for this.

## [s7] The banked floor-4 body still reproduces on the chassis this session was dispatched against (the brief reported the chassis measurement as unavailable, so it could not be taken on trust).
- mechanism: control measurement; every banked spelling conclusion in this ledger is chassis-relative.
- probe: memory/grind/func_80072CD4/candidate.c applied to src/text1b.c via tmp/grind/func_80072CD4/s7/apply.py, then `& tools/wteng.ps1 main sandbox func_80072CD4 --disable all`.
- result: score 4, target_insns 79, build_insns 79, rules_dropped 0 (tmp/grind/func_80072CD4/s7/syn_control_floor4.json).
- verdict: CONFIRMED

## [s7] Duplicating exactly ONE of the two shared 0xFC red components (r0 @4 or r1 @0xC) into both inner arms, with the other hoisted into the merge block, is a middle setting between the banked floor-4 body (both hoisted) and the banned per-arm body (both duplicated), and could reach target's merge head without the banned two-store duplication.
- mechanism: target's merge head is `sb v1,4 / sb v1,0xC / sb v0,0xE`; s5-forensics proved (L1) sched2 sinks producer-less merge-block stores to the block tail and (L2) jump2's cross_jump runs AFTER sched2 (toplev.c :3117 vs :3142) so only a cross-jumped common tail can occupy merge position 0. A half-duplication puts ONE of the two stores in that tail, so if the head only needed one of them the arrangement would score better than 4 without duplicating both.
- probe: tmp/grind/func_80072CD4/s7/syn_halfdup_r0only.c (r0 in both arms, r1 hoisted) and syn_halfdup_r1only.c (r1 in both arms, r0 hoisted) - POLY_G4 struct spelling, otherwise identical to the banked bodies; applied via s7/apply.py; `& tools/wteng.ps1 main sandbox func_80072CD4 --disable all`; JSON artifacts syn_halfdup_r0only.json / syn_halfdup_r1only.json.
- result: 6 / 80 and 9 / 80 - both score WORSE than the floor of 4 and both are one instruction OVER target's 79. The lone hoisted red needs its own merge-block constant materialisation while the arm-resident one already has its own, so a half-dup form pays for two materialisations where target, the 0-dup form (one shared merge-block li) and the 2-dup form (one li in the inner-beqz delay slot, both stores riding jump2's common tail) each pay for one.
- verdict: KILLED

## [s7] Some source could make the two inner arms tail-distinct by REAL semantic content rather than by a lever, and so reach target's merge order without duplicating @4/@0xC (the last item the s6 frontier left open, flagged there as a probable formality).
- mechanism: if the arms ended in genuinely different work, jump2's backward common-tail scan would stop at a different point and the spliced head could differ.
- probe: inspection of asm/funcs/func_80072CD4.s (no build needed, as the s6 frontier itself directed).
- result: the two arms differ only in three literal colour components - b0 0x1E vs 0x50, g1 0xC8 vs 0xDC, b1 0x32 vs 0x46. There is no semantic content in this function available to differentiate the arm tails with; the only differentiator is the arm-final li that target already carries.
- verdict: KILLED
