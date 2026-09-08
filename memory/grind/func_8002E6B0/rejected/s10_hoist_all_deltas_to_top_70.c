/* s10 n-family: all three blocks deltas computed at top */
s32 func_8002E6B0(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3)
{
  s32 center_x = ((arg0[0] + arg1[0]) + arg2[0]) / 3;
  s32 center_z = ((arg0[2] + arg1[2]) + arg2[2]) / 3;
  s32 cross_center;
  s32 cross_point;
  s32 dz1 = arg1[2] - arg0[2];
  s32 dx1 = arg1[0] - arg0[0];
  s32 dz2 = arg2[2] - arg0[2];
  s32 dx2 = arg2[0] - arg0[0];
  s32 dz3 = arg2[2] - arg1[2];
  s32 dx3 = arg2[0] - arg1[0];
  s32 ret = 0;
  {
    cross_center = (dz1 * (center_x - arg0[0])) - (dx1 * (center_z - arg0[2]));
    cross_point = (dz1 * (arg3[0] - arg0[0])) - (dx1 * (arg3[2] - arg0[2]));
  }
  if ((cross_center ^ cross_point) < 0)
  {
    goto end;
  }
  {
    cross_center = ((ret = dz2) * (center_x - arg0[0])) - (dx2 * (center_z - arg0[2]));
    cross_point = (dz2 * (arg3[0] - arg0[0])) - (dx2 * (arg3[2] - arg0[2]));
    ret = 0;
  }
  if ((cross_center ^ cross_point) < 0)
  {
    goto end;
  }
  {
    cross_center = (dz3 * (center_x - arg1[0])) - (dx3 * (center_z - arg1[2]));
    cross_point = (dz3 * (arg3[0] - arg1[0])) - (dx3 * (arg3[2] - arg1[2]));
  }
  ret = (cross_center ^ cross_point) >= 0;
  end:
  return ret;
}
