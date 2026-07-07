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

## [s3] H2: rewriting the outer/inner comparisons using `D_800A38F8 <= D_800A37A0` (instead of `!(D_800A38F8 > D_800A37A0)`) preserves score=0 with cc1 output byte-identical to H1.
- mechanism: u8 globals => unsigned comparison. `a <= b` = `!(a > b)` is a jump.c-level canonicalization: same sltu operand order, same bnez/beq sense, same delay-slot candidates. Confirmed by cc1 dump: H2.func.s diff HEAD.func.s shows the SAME six-line jump-label renumbering as H1 (.L272<->.L271, .L273<->.L272, .L277<->.L276); no instruction/register/operand differences. diff H1.func.s H2.func.s => 0 lines.
- probe: tmp/grind/camera_SetMatrix_8001DBE4/s3/build.sh cc1-dumps each variant using Makefile CC_FLAGS; extract.py slices out camera_SetMatrix_8001DBE4 body; diff HEAD.func.s H2.func.s and diff H1.func.s H2.func.s.
- result: diff HEAD vs H2 = 6 lines label-renumber only. diff H1 vs H2 = 0 lines. Sandbox=0 for H2 is proven by construction.
- verdict: CONFIRMED

## [s3] H4: replacing `i += 1;` with `i++;` in the second do-while (with H1's `!(> )` outer/inner form) preserves score=0 with cc1 output byte-identical to H1.
- mechanism: GCC 2.7.2 treats `i += 1` and `i++` as identical on a plain s32 local at -O2 (both lower to a single addu/tree with the same value use). Confirmed: diff H1.func.s H4.func.s => 0 lines; diff HEAD.func.s H4.func.s = SAME six-line label renumber as H1/H2.
- probe: Same build.sh + extract.py pipeline as H2 above.
- result: diff HEAD vs H4 = 6 lines label-renumber only. diff H1 vs H4 = 0 lines. Sandbox=0 for H4 is proven by construction.
- verdict: CONFIRMED

## [s3] The reintegration lock is still held by session c56ff61a-f40f-49f8-ad83-348536c53c90 (the same holder that blocked s2).
- mechanism: tmp/.main_reintegration.lock file present with the s2 holder's session_id; PreToolUse main_reintegration_lock hook BLOCKED Edit of src/code6cac.c on the H1-apply attempt, reporting 11 minutes held. Role rule forbids acquire/release/steal from this session, even against a stale-looking lock.
- probe: Read tmp/.main_reintegration.lock (holder=c56ff61a, ts=1783399525, head=0c2d3c91); attempt Edit src/code6cac.c line 1618 replacing empty-if with H1; observe the hook block.
- result: Edit blocked; recorded per role and moved on to out-of-tree structural work.
- verdict: CONFIRMED
