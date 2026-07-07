# Hypothesis ledger — camera_SetMatrix_8001DBE4

## [s1] Rewriting HEAD's `if (D_800A38F8 > D_800A37A0) {} else { LOOPS }` as `if (!(D_800A38F8 > D_800A37A0)) { LOOPS }` preserves distance 0.
- mechanism: GCC 2.7.2 jump.c routinely inverts if-then-goto branch sense: `if (!c) body;` emits `if (c) goto skip; body; skip:` — identical control-flow shape as HEAD's empty-then/else-body form. Same sltu operand order (D_800A37A0<D_800A38F8), same bnez, same delay-slot candidates (initial skip = nothing available, loop1 exit = i=0 init addu).
- probe: Apply candidate.c to src/code6cac.c line 1611; run `& tools/wteng.ps1 main sandbox camera_SetMatrix_8001DBE4 --disable all`; expect score 0.
- result: not run (recon modality — deferred to next session)
- verdict: 

## [s2] H1: rewriting HEAD's `if (D_800A38F8 > D_800A37A0) {} else { LOOPS }` as `if (!(D_800A38F8 > D_800A37A0)) { LOOPS }` preserves score=0 with byte-identical cc1 output.
- mechanism: GCC 2.7.2 jump.c invert_jump on if-then-goto skeleton: same sltu operand order (comparison expression identical, only enclosing sense negated), same bnez branch sense, same scheduling candidates. Confirmed by cc1 dump comparison: HEAD vs H1 asm bodies differ only in .L jump-label numbering (.L272↔.L271, .L273↔.L272, .L277↔.L276); every instruction, register, operand, and PC offset match after assembly.
- probe: 1) Sandbox on HEAD (empty-if form): score=0, 51/51 insns. 2) Attempt Edit src/code6cac.c:1618-1632 → BLOCKED by main_reintegration_lock (holder c56ff61a-f40f-49f8-ad83-348536c53c90, held for entire session). Per session role never touched the lock. 3) Out-of-tree cc1 pipeline: copied src/code6cac.c to tmp/grind/.../s2/{HEAD,H1}.c, applied H1 rewrite via python replace, ran cpp+cc1 (matched build's CC_FLAGS -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin), extracted camera_SetMatrix_8001DBE4 function bodies from each .s, ran diff.
- result: cc1 dump diff = 6 lines, all jump-label renumbering; no instruction/register/operand differences. Sandbox=0 for H1 is proven by construction (post-assembly bytes will be identical).
- verdict: CONFIRMED
