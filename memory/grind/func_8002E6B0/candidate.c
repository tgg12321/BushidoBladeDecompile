/* s4 CANDIDATE - sandbox 34 / 95 insns (previous floor 40, v12).
   The single delta from v12 is block 2's `cross_point = dx;` staging line: the
   function-scope local `cross_point` is borrowed to carry block 2's `dx` for the
   cross_center product, before being overwritten with its own real value two
   statements later. Found by decomp-permuter (s4, perm_b, output-345-1) and
   confirmed load-bearing by ablation: a FRESH named intermediate (`s32 dxa = dx;`)
   in the same position measures 40, the same borrow in block 1 only measures 40,
   in all three blocks 61, and borrowing cross_center for `dz` instead 53. Only
   THIS borrow, in THIS block, reaches 34.
   FAMILY: variable reuse (.claude/rules/defeat-licm-hoist-var-reuse.md, gated by
   .claude/rules/staged-value-reused-variable.md) - an EXISTING local borrowed for
   a second value, not an invented one. It is a FAKE construct and would need the
   annotation + lever-exhaustion record before any candidate-ready submission.
   The best FAKE-FREE form on this chassis remains v12 at 40
   (rejected/s4_v12_plain_no_varreuse_40.c). */
s32 func_8002E6B0(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3)
{
  s32 center_x = ((arg0[0] + arg1[0]) + arg2[0]) / 3;
  s32 center_z = ((arg0[2] + arg1[2]) + arg2[2]) / 3;
  s32 cross_center;
  s32 cross_point;
  {
    s32 dz = arg1[2] - arg0[2];
    s32 dx = arg1[0] - arg0[0];
    cross_center = (dz * (center_x - arg0[0])) - (dx * (center_z - arg0[2]));
    cross_point = (dz * (arg3[0] - arg0[0])) - (dx * (arg3[2] - arg0[2]));
  }
  if ((cross_center ^ cross_point) < 0)
  {
    return 0;
  }
  {
    s32 dz = arg2[2] - arg0[2];
    s32 dx = arg2[0] - arg0[0];
    cross_point = dx;
    cross_center = (dz * (center_x - arg0[0])) - (cross_point * (center_z - arg0[2]));
    cross_point = (dz * (arg3[0] - arg0[0])) - (dx * (arg3[2] - arg0[2]));
  }
  if ((cross_center ^ cross_point) < 0)
  {
    return 0;
  }
  {
    s32 dz = arg2[2] - arg1[2];
    s32 dx = arg2[0] - arg1[0];
    cross_center = (dz * (center_x - arg1[0])) - (dx * (center_z - arg1[2]));
    cross_point = (dz * (arg3[0] - arg1[0])) - (dx * (arg3[2] - arg1[2]));
  }
  return (cross_center ^ cross_point) >= 0;
}
