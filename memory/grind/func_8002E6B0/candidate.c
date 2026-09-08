/* s5 CANDIDATE - sandbox 26 / 96 insns (previous floor 34, s4's cross_point borrow).
   Chassis: the s4 "ret-var + goto end" exit form (s32 ret = 0; both early exits
   `goto end;`, tail `ret = (cc ^ cp) >= 0; end: return ret;`) PLUS one borrow in
   block 2: block 2's `dz` is staged through the return carrier `ret`
   (`((ret = dz) * (center_x - arg0[0]))`), and `ret` is restored to its real
   value 0 at the end of block 2 so the second early exit still returns 0.

   WHY IT WORKS (measured, not inferred): s4 had the two halves of the residual
   anti-correlated - the block-2 cross_point borrow bought the target's SIXTH
   callee-save ($s5) but not the exit shape, and the ret-var chassis bought the
   exit shape but dropped back to five callee-saves because the ret pseudo
   absorbed the block-1/2 pressure the borrow created. Staging dz through the ret
   pseudo ITSELF re-creates that pressure inside the ret-var chassis, so both
   halves are held at once. The -dz side-by-side now reproduces the target's exit
   structure exactly (`move v0,zero` in the first bltz's delay slot, BOTH bltz
   straight to the epilogue, no `j`, no orphan zero block) AND the sixth
   callee-save (`sw s5,20(sp)` / `mflo s5`).

   RESIDUAL (2 insns + a register permutation): our ret pseudo lands in $v1 rather
   than $v0, which costs a trailing `move v0,v1`, and the `ret = 0;` restore costs
   a `move v1,zero` in the second bltz's delay slot (the target needs no restore
   because its $v0 is simply never clobbered across block 2). Everything else is a
   consistent rename of {v0,v1,a0,a1,a2}.

   ORIGIN: decomp-permuter (s5, perm_d, output-265-1) proposed the staging but in a
   SEMANTICALLY BROKEN form (no restore, so the second early exit would return dz
   instead of 0 - the permuter does not check semantics); the repair is the
   `ret = 0;` restore. The broken form measures 25.

   FAMILY: variable reuse - an EXISTING local (the real return carrier) borrowed to
   stage a real, immediately-consumed value, then written back with its own real
   value (.claude/rules/defeat-licm-hoist-var-reuse.md, gated by
   .claude/rules/staged-value-reused-variable.md). It is a FAKE construct: any
   candidate-ready must carry the /* FAKE: what + mechanism + lever-exhaustion */
   annotation and quote that rule's scope sentence verbatim. The best FAKE-FREE
   form on this chassis remains v12 at 40 (rejected/s4_v12_plain_no_varreuse_40.c).

   TIES AT 26: d2 (this body plus s4's cross_point=dx borrow AND the ret staging in
   both block-2 products), e6, e1 (ret declared first), i1/i2 (block 2's
   `center_z - arg0[2]` additionally staged through dx, recomputed or restored). */
s32 func_8002E6B0(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3)
{
  s32 center_x = ((arg0[0] + arg1[0]) + arg2[0]) / 3;
  s32 center_z = ((arg0[2] + arg1[2]) + arg2[2]) / 3;
  s32 cross_center;
  s32 cross_point;
  s32 ret = 0;
  {
    s32 dz = arg1[2] - arg0[2];
    s32 dx = arg1[0] - arg0[0];
    cross_center = (dz * (center_x - arg0[0])) - (dx * (center_z - arg0[2]));
    cross_point = (dz * (arg3[0] - arg0[0])) - (dx * (arg3[2] - arg0[2]));
  }
  if ((cross_center ^ cross_point) < 0)
  {
    goto end;
  }
  {
    s32 dz = arg2[2] - arg0[2];
    s32 dx = arg2[0] - arg0[0];
    cross_center = ((ret = dz) * (center_x - arg0[0])) - (dx * (center_z - arg0[2]));
    cross_point = (dz * (arg3[0] - arg0[0])) - (dx * (arg3[2] - arg0[2]));
    ret = 0;
  }
  if ((cross_center ^ cross_point) < 0)
  {
    goto end;
  }
  {
    s32 dz = arg2[2] - arg1[2];
    s32 dx = arg2[0] - arg1[0];
    cross_center = (dz * (center_x - arg1[0])) - (dx * (center_z - arg1[2]));
    cross_point = (dz * (arg3[0] - arg1[0])) - (dx * (arg3[2] - arg1[2]));
  }
  ret = (cross_center ^ cross_point) >= 0;
  end:
  return ret;

}
