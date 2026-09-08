s32 func_8002E6B0(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3)
{
  s32 t2 = arg0[0];
  s32 t4 = arg1[0];
  s32 t6 = arg2[0];
  s32 t1 = arg0[2];
  s32 t3 = arg1[2];
  s32 t5 = arg2[2];
  s32 s1 = ((t2 + t4) + t6) / 3;
  s32 v0 = t3 - t1;
  s32 t7 = s1 - t2;
  s32 s3 = arg3[0];
  s32 t8 = s3 - t2;
  s32 s2 = arg3[2];
  s32 t0 = s2 - t1;
  s32 v0b = t4 - t2;
  s32 s0 = ((t1 + t3) + t5) / 3;
  s32 a1 = s0 - t1;
  s32 ret = 0;
  if ((((v0 * t7) - (v0b * a1)) ^ ((v0 * t8) - (v0b * t0))) >= 0)
  {
    s32 a0 = t5 - t1;
    s32 v1 = t6 - t2;
    if ((((a0 * t7) - (v1 * a1)) ^ ((a0 * t8) - (v1 * t0))) >= 0)
    {
      s32 a0b = t5 - t3;
      s32 v1b = t6 - t4;
      ret = (((a0b * (s1 - t4)) - (v1b * (s0 - t3))) ^ ((a0b * (s3 - t4)) - (v1b * (s2 - t3)))) >= 0;
    }
  }
  return ret;
}
