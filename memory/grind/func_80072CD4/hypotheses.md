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
