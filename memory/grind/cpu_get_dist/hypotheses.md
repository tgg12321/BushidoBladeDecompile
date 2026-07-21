# Hypothesis ledger — cpu_get_dist

## s1 (recon, 2026-07-21) — initial frontier

1. **Directed permuter re-seeded from the 15-form** (ledger-mandated next modality; both prior campaigns seeded from the 21-base and found only cheat-forms). Mechanism: the target allocation is reachable (a permuter byte-match exists but was cheat-spelled); the clean lever may lie in mutation space around the closer 15 base. Probe: tools/permuter_campaign.py with target.o built from asm/funcs/cpu_get_dist.s + prelude, base = 15-form; in-turn wait + harvest per fresh-seed discipline.
2. **INSN_PRIORITY deepening in the 15-space**: give lh cos (insn 59) a strictly deeper nearest-successor chain than the class-3 loads so priority (not class) decides. The 21-form proof of symmetry (both feed the first mult) does not automatically transfer to staged/nested restructurings in the rz-first space. Probe: staged expressions reusing the SAME variable (avoid v21-style extra-pseudo regression), measured against the fresh s1 .sched dump.
3. **Density/nop-fill axis**: target keeps 4-5 mult-latency nops the our-build scheduler fills (68 vs 63 insns). Any form that shrinks the ready set at the mflo/mult hazard boundaries (fewer independent movable insns at those points) pushes our stream toward target's sparser schedule. Probe: count nop provenance (maspsx/multu_pad vs sched) in build vs target, then test statement placements that starve the filler slots.

## [s1] cpu_get_dist has an independent COMPLETED-C near-clone (func_8003032C, similarity 1.000) whose C body can seed the match
- mechanism: find_duplicates.py leads pair INCOMPLETE functions with completed analogs by opcode-class Levenshtein
- probe: Re-ran tools/find_duplicates.py fresh; inspected asm/funcs/func_8003032C.s and named_syms.txt
- result: func_8003032C.s is a stale same-address twin of cpu_get_dist itself (both 0x8003032C; rename artifact, named_syms.txt:660). No independent analog exists; lead is self-referential.
- verdict: KILLED

## [s1] Within the rz-first 15-form, writing rz's inner sum in the target/m2c addend order ((-vx)*sin first) keeps or beats floor 15
- mechanism: Target's addu for the rz sum consumes (-vx)*sin (reg a2) as the first operand; matching source addend order could align combine/sched emission
- probe: Edited src to rz = -((((-vx)*sin_val) + (vz*cos_val)) >> 12); ran sandbox --disable all
- result: Score 25 (build 61 vs target 62) — regression from 15. The vz*cos lead is load-bearing. Banked as rejected/rz-first-negvx-sin-lead-25.c
- verdict: KILLED

## [s1] The ledger's imported floor 15 for candidate.c reproduces on current main
- mechanism: Baseline validity check (recon mandate)
- probe: Applied candidate.c to src/code6cac_b.c; ran canonical + sandbox --disable all
- result: canonical: verdict C, distance 15; sandbox: 15 (build 63 vs target 62, 4 rules dropped, cheat-asm stripped). Exact reproduction.
- verdict: CONFIRMED
