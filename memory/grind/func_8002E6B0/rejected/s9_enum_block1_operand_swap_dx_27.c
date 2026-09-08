/* s9 (enumerate). Block 1 with the dx-side products written operand-swapped
   ((center_z - arg0[2]) * dx instead of dx * (center_z - arg0[2])). Scores 27
   - strictly WORSE than the 26 floor. Banked as the representative of the
   448-spelling commutative-swap sweeps (enumAs / enumBs / enumCs): the swap
   axis on all three blocks produced 26 at best and never below. */
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
    cross_center = (dz * (center_x - arg0[0])) - ((center_z - arg0[2]) * dx);
    cross_point = (dz * (arg3[0] - arg0[0])) - ((arg3[2] - arg0[2]) * dx);
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
