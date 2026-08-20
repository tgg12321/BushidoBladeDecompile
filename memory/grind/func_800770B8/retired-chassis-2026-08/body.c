void func_800770B8(s32 arg0_in, s32 arg1, long arg2)
{
  register s32 arg0 asm("$16") = arg0_in;
  u16 sp[2];
  s32 *p_old;
  u8 *p;
  s32 r;
  s16 t0;
  s16 a2;
  p_old = (s32 *) (arg0 + 0x58);
  __asm__ volatile("" : : "r"(arg1) : "memory");
  sp[0] = 0;
  sp[1] = 0;
  ClearOTagR(D_800A374C, 0x1008);
  D_800A35D8 = arg0;
  snd_StopAll();
  func_8006E950(6, p_old);
  r = func_80076FF8(p_old);
  {
    s32 *prev = p_old;
    p_old = (s32 *) func_8006E49C(r, D_800A35D8);
    D_800A36A0 = (u8 *) p_old;
    *((s32 *) ((u8 *) p_old + 4)) = (s32) prev;
    *((s32 *) ((u8 *) p_old + 0x30)) = 0;
    *((s16 *) ((u8 *) p_old + 0x34)) = 0;
    p = (u8 *) p_old;
  }
  t0 = 0;
  a2 = 0;
  do
  {
    u8 *base = D_800A36A0;
    *((s16 *) ((base + (t0 * 2)) + 0x10)) = 0;
    *((s16 *) ((base + (t0 * 2)) + 0x8)) = 0;
    *((s16 *) ((base + (t0 * 2)) + 0xC)) = 0;
    *((s16 *) ((base + (t0 * 2)) + 0x14)) = 0;
    *((s16 *) ((base + (t0 * 2)) + 0x3C)) = 0;
    {
      s16 *p_d0 = &(&D_800A35D0)[t0 * 2];
      p_d0[1] = 0;
      p_d0[0] = 0;
    }
    *((s16 *) ((base + (t0 * 4)) + 0x42)) = 0;
    *((s16 *) ((base + (t0 * 4)) + 0x40)) = 0;
    *((u8 *) ((base + t0) + 0x68)) = (u8) t0;
    {
      s16 *p_6a = (s16 *) ((D_800A36A0 + (t0 * 10)) + 0x6A);
      s16 *p_7e = (s16 *) ((D_800A36A0 + (t0 * 10)) + 0x7E);
      do
      {
        p_6a[a2] = -1;
        p_7e[a2] = 0;
        a2 = (s16) (a2 + 1);
      }
      while (a2 < 5);
    }
    a2 = 0;
    *((s16 *) ((D_800A36A0 + (t0 * 2)) + 0x5C)) = 0;
    *((s16 *) ((D_800A36A0 + (t0 * 2)) + 0x60)) = 5;
    do
    {
      s16 idx = (s16) (a2 + (t0 * 10));
      (&D_8009BCE4)[idx] = (u8) ((&D_8009BCE4)[idx] & 0xF2);
      if ((arg2 & (1 << idx)) != 0)
      {
        (&D_8009BCE4)[idx] = (u8) ((&D_8009BCE4)[idx] | 1);
        sp[t0] += 1;
      }
      a2 = (s16) (a2 + 1);
    }
    while (a2 < 0xA);
    t0 = (s16) (t0 + 1);
    a2 = 0;
  }
  while (t0 < 2);
  p = D_800A36A0;
  *((s32 *) (p + 0x20)) = 0;
  *((s32 *) (p + 0x1C)) = 0;
  if (((s16) sp[0]) < ((s16) sp[1]))
  {
    *((s8 *) (p + 0x64)) = (s8) (((s16) sp[0]) - 3);
  }
  else
  {
    *((s8 *) (p + 0x64)) = (s8) (((s16) sp[1]) - 3);
  }
  if ((*((u8 *) (D_800A36A0 + 0x64))) >= 3)
  {
    *((u8 *) (D_800A36A0 + 0x64)) = 2;
  }
  p = D_800A36A0;
  *((s32 *) p) = arg1;
  *((s8 *) (p + 0x65)) = 0;
  p = D_800A36A0;
  *((u8 *) (p + 0x67)) = 1;
  p = D_800A36A0;
  *((u8 *) (p + 0x66)) = (&D_8009BD21)[(*((u8 *) (p + 0x67))) * 2];
  arg0 = 1;
  D_800A35DC = arg0;
}
