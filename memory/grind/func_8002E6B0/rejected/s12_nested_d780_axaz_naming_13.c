/* s12 REJECTED: 13/94 - correct nested chassis, but naming the centroid differences ax/az hoists their subu to the function top and breaks the mult/mflo interleave */
s32 func_8002E6B0(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3)
{
  s32 center_x = ((arg0[0] + arg1[0]) + arg2[0]) / 3;
  s32 center_z = ((arg0[2] + arg1[2]) + arg2[2]) / 3;
  s32 cross_center;
  s32 cross_point;
  {
    s32 ax = center_x - arg0[0];
    s32 dz = arg1[2] - arg0[2];
    s32 az = center_z - arg0[2];
    cross_center = dz * ax - (arg1[0] - arg0[0]) * az;
    cross_point = dz * (arg3[0] - arg0[0]) - (arg1[0] - arg0[0]) * (arg3[2] - arg0[2]);
  }
  if ((cross_center ^ cross_point) >= 0)
  {
  {
    s32 ax = center_x - arg0[0];
    s32 dz = arg2[2] - arg0[2];
    s32 az = center_z - arg0[2];
    cross_center = dz * ax - (arg2[0] - arg0[0]) * az;
    cross_point = dz * (arg3[0] - arg0[0]) - (arg2[0] - arg0[0]) * (arg3[2] - arg0[2]);
  }
  if ((cross_center ^ cross_point) >= 0)
  {
  {
    s32 ax = center_x - arg1[0];
    s32 dz = arg2[2] - arg1[2];
    s32 az = center_z - arg1[2];
    cross_center = dz * ax - (arg2[0] - arg1[0]) * az;
    cross_point = dz * (arg3[0] - arg1[0]) - (arg2[0] - arg1[0]) * (arg3[2] - arg1[2]);
  }
  if ((cross_center ^ cross_point) >= 0)
  {
    return 1;
  }
  }
  }
  return 0;
}
