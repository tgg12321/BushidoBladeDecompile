# Hypothesis ledger — rob_life_ctrl_2

## [s1] HEAD body with regfix rules stripped scores 2 at the honest sandbox --disable all metric (nothing has drifted vs ledger s0 floor=2)
- mechanism: sandbox --disable all: rules_dropped=2, cheat_asm_stripped=78, build_insns=96, target_insns=96, score=2. canonical: verdict=C, distance=2 (pure-C target).
- probe: & tools/wteng.ps1 main sandbox rob_life_ctrl_2 --disable all; & tools/wteng.ps1 main canonical rob_life_ctrl_2
- result: score=2; verdict=C; the 2 stripped rules are the rob_life_ctrl_2 mflo/sra substs at regfix.txt:487-488
- verdict: CONFIRMED

## [s1] The exact residual is the t-mult mflo destination and its dependent sra source; target reuses a1(=4096-constant) for t's mflo after a1's last subu use at 0xB0 dies, our build assigns pseudo 104 to $8/t0 instead
- mechanism: objdump of tmp/sandbox/rob_life_ctrl_2/text1a_c.o vs asm/funcs/rob_life_ctrl_2.s: at 0xC4 target `mflo a1` / ours `mflo t0`; at 0xCC target `sra a1,a1,0xc` / ours `sra a1,t0,0xc`. All other 94 insns match byte-for-byte. Precisely matches the ledger's documented gap.
- probe: mipsel-linux-gnu-objdump -d tmp/sandbox/rob_life_ctrl_2/text1a_c.o | sed -n '/<rob_life_ctrl_2>:/,/^$/p'; compare against asm/funcs/rob_life_ctrl_2.s
- result: diff = { 0xC4: mflo $8 -> $5, 0xCC: sra $5,$8,12 -> sra $5,$5,12 } = 2 insns; 94/96 already byte-identical
- verdict: CONFIRMED

## [s2] Hoisting the outer subu of t into a named block-local `s32 tsub = 0x1000 - ((s*(0x1000-f))>>12); t = (v*tsub)>>12;` will change the t-mult pseudo's death/birth window and let RA reuse a1 for the mflo.
- mechanism: Frontier hypothesis 2 (from s1 ledger): a named intermediate for the outer subu could shift pseudo 104's LUID/priority tiebreaker.
- probe: Applied block-scoped `s32 tsub = ...` form; & tools/wteng.ps1 main sandbox rob_life_ctrl_2 --disable all; objdump the resulting text1a_c.o.
- result: score=2 unchanged; objdump insns at 0xC4/0xCC still `mflo t0` + `sra a1,t0,0xc` (same 2-insn diff). Full byte-diff empty vs HEAD compile. tmp/grind/rob_life_ctrl_2/s2/probe1_tsub.txt.
- verdict: KILLED

## [s2] Naming the inner subu `s32 nf = 0x1000 - f;` and using `(s*nf)` inside t will kill the second literal `0x1000` reference and force a1's live range to end at t's outer subu, freeing a1 for pseudo 104.
- mechanism: Frontier hypothesis 2 variant: a block-local intermediate for `0x1000 - f` explicitly extracts the CSE that target already exhibits at target 0x32D8C (`subu v0,a1,v0`).
- probe: Applied `{ s32 nf = 0x1000 - f; q = ...; t = (v*(0x1000-((s*nf)>>12)))>>12; }`; sandbox + objdump.
- result: score=2 unchanged; objdump bytes IDENTICAL to HEAD (probe2_nf.txt). GCC's CSE already extracts `0x1000-f` at RTL level — the source-level naming is invisible.
- verdict: KILLED

## [s2] Precomputing the full inner `s32 tinner = (s*(0x1000-f))>>12;` BEFORE q reorders the scheduling of the two mults and gives pseudo 104 a different birth-time / conflict set.
- mechanism: A named intermediate whose value crosses q's computation could perturb LUID enough to flip the priority tiebreaker at the mflo point.
- probe: Applied `{ s32 tinner = (s*(0x1000-f))>>12; q = ...; t = (v*(0x1000-tinner))>>12; }`; sandbox + objdump; diff vs probe2.
- result: score=2 unchanged; objdump bytes IDENTICAL to probe2 (empty diff). GCC folds the block-local intermediate into the same DAG as HEAD.
- verdict: KILLED

## [s2] Moving `i = h >> 12;` past the q/t block (LUID perturbation for the switch's dispatch pseudo) will change the relative pseudo ordering enough to flip the priority tiebreaker for pseudo 104.
- mechanism: Register-allocation Lever B family (per register-alloc-pure-c.md): change LUID order of a nearby local to perturb the allocno priority queue.
- probe: Moved `i = h >> 12;` to after the `t = ...` statement; sandbox + objdump; diff vs HEAD.
- result: score=2 unchanged; objdump bytes IDENTICAL to HEAD. GCC's tree-level scheduling of the h-shifts is invariant to source order for this single-use LUID.
- verdict: KILLED
