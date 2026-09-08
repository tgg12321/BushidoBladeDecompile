s32 func_8002E6B0(u8 *arg0, u8 *arg1, u8 *arg2, u8 *arg3)
{
  s32 center_x = ((*(s32 *)(arg0) + *(s32 *)(arg1)) + *(s32 *)(arg2)) / 3;
  s32 center_z = ((*(s32 *)(arg0 + 8) + *(s32 *)(arg1 + 8)) + *(s32 *)(arg2 + 8)) / 3;
  s32 cross_center;
  s32 cross_point;
  s32 ret = 0;
  {
    s32 dz = *(s32 *)(arg1 + 8) - *(s32 *)(arg0 + 8);
    s32 dx = *(s32 *)(arg1) - *(s32 *)(arg0);
    cross_center = (dz * (center_x - *(s32 *)(arg0))) - (dx * (center_z - *(s32 *)(arg0 + 8)));
    cross_point = (dz * (*(s32 *)(arg3) - *(s32 *)(arg0))) - (dx * (*(s32 *)(arg3 + 8) - *(s32 *)(arg0 + 8)));
  }
  if ((cross_center ^ cross_point) < 0)
  {
    goto end;
  }
  {
    s32 dz = *(s32 *)(arg2 + 8) - *(s32 *)(arg0 + 8);
    s32 dx = *(s32 *)(arg2) - *(s32 *)(arg0);
    cross_center = ((ret = dz) * (center_x - *(s32 *)(arg0))) - (dx * (center_z - *(s32 *)(arg0 + 8)));
    cross_point = (dz * (*(s32 *)(arg3) - *(s32 *)(arg0))) - (dx * (*(s32 *)(arg3 + 8) - *(s32 *)(arg0 + 8)));
    ret = 0;
  }
  if ((cross_center ^ cross_point) < 0)
  {
    goto end;
  }
  {
    s32 dz = *(s32 *)(arg2 + 8) - *(s32 *)(arg1 + 8);
    s32 dx = *(s32 *)(arg2) - *(s32 *)(arg1);
    cross_center = (dz * (center_x - *(s32 *)(arg1))) - (dx * (center_z - *(s32 *)(arg1 + 8)));
    cross_point = (dz * (*(s32 *)(arg3) - *(s32 *)(arg1))) - (dx * (*(s32 *)(arg3 + 8) - *(s32 *)(arg1 + 8)));
  }
  ret = (cross_center ^ cross_point) >= 0;
  end:
  return ret;
}
