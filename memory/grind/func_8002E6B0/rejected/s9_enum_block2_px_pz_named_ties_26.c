/* s9 (enumerate). Block-2 spelling with the two arg3-relative point deltas
   named (px / pz) instead of inlined, block 1 carrying the ax/az hoist.
   Scores 26 - a BYTE TIE with candidate.c, not an improvement. Banked as the
   representative of the 130-spelling enumC/enumCp sweeps (declaration set x
   declaration order x assignment order over dz/dx/ax/az and dz/dx/px/pz):
   every one of those 260 spellings scored 26 or 43, never below 26. */
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
    s32 ax = center_x - arg0[0];
    s32 az = center_z - arg0[2];
    cross_center = (dz * ax) - (dx * az);
    cross_point = (dz * (arg3[0] - arg0[0])) - (dx * (arg3[2] - arg0[2]));
  }
  if ((cross_center ^ cross_point) < 0)
  {
    goto end;
  }
  {
    s32 dz = arg2[2] - arg0[2];
    s32 dx = arg2[0] - arg0[0];
    s32 px = arg3[0] - arg0[0];
    s32 pz = arg3[2] - arg0[2];
    cross_center = ((ret = dz) * (center_x - arg0[0])) - (dx * (center_z - arg0[2]));
    cross_point = (dz * px) - (dx * pz);
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
