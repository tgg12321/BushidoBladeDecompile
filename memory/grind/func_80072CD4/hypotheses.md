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
