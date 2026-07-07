# Hypothesis ledger — camera_SetMatrix_8001DBE4

## [s1] Rewriting HEAD's `if (D_800A38F8 > D_800A37A0) {} else { LOOPS }` as `if (!(D_800A38F8 > D_800A37A0)) { LOOPS }` preserves distance 0.
- mechanism: GCC 2.7.2 jump.c routinely inverts if-then-goto branch sense: `if (!c) body;` emits `if (c) goto skip; body; skip:` — identical control-flow shape as HEAD's empty-then/else-body form. Same sltu operand order (D_800A37A0<D_800A38F8), same bnez, same delay-slot candidates (initial skip = nothing available, loop1 exit = i=0 init addu).
- probe: Apply candidate.c to src/code6cac.c line 1611; run `& tools/wteng.ps1 main sandbox camera_SetMatrix_8001DBE4 --disable all`; expect score 0.
- result: not run (recon modality — deferred to next session)
- verdict: 
