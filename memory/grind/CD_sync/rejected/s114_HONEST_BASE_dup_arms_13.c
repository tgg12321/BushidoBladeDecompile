/* s114 NEW HONEST BASE - NOT a dead form. Duplicated-statement-into-arms: the whole do_timeout
   block (puts + the two idx_1494 byte reads + both tbl_125c address computations + debug_printf +
   cdrom_ClearIrq + v0 = -1) is duplicated into BOTH do_timeout arrival arms, and jump2's
   find_cross_jump re-merges the two copies to identical bytes: build_insns 160, rules_dropped 0,
   score 13 (vs the n3 honest base's 15).
   ALLOCDBG ord=11..16: p77 idx_1494 9/190/1421 -> $s1, p80 2/21/952 -> $s2,
   p79 tbl_125c 5/188/531 -> $s3 (TARGET), p78 idx_1495 2/95/210 -> $s4 (TARGET),
   p72 mode 2/96/208 -> $s5 (TARGET), p73 result 2/99/202 -> $s6 (TARGET).
   Four of the five target s-register seats land for the first time on an honest chassis; the sole
   residual is the p77/p80 inversion at $s1/$s2, caused by the duplication also lifting p77's
   loop-weighted refs from 14 to 27 (pri 933 -> 1421, above p80's 952). */
s32 CD_sync(s32 a0, u8 *a1)
{
  int new_var;
  s32 v0;
  s32 cnt;
  u8 *idx_1494;
  u8 *idx_1495;
  s32 *tbl_125c;
  u8 saved;
  s32 status;
  u8 *src;
  u8 *dst;
  s32 i;
  u8 b;
  s32 temp;
  D_800F19B8 = sys_VSync(-1) + 0x3C0;
  tbl_125c = D_800A125C;
  idx_1494 = &D_800A1494;
  D_800F19BC = 0;
  D_800F19C0 = &D_80016240;
  loop:
  v0 = sys_VSync(-1);

  if (D_800F19B8 < v0)
  {
    tslTm2LoadImage_2(&D_800161B8);
    {
      s32 arg5;
      s32 t0;
      s32 ix;
      void **pp;
      t0 = idx_1494[0];
      ix = idx_1494[1];
      pp = (void **)&D_800F19C0;
      t0 *= 4;
      t0 = (s32)((u8 *)tbl_125c + t0);
      ix <<= 2;
      arg5 = *(s32 *)(ix + (s32)tbl_125c);
      debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);
    }
    cdrom_ClearIrq();
    v0 = -1;
    goto check;
  }
  cnt = D_800F19BC;
  D_800F19BC = cnt - -1;
  if (0x3C0000 < cnt)
  {
    tslTm2LoadImage_2(&D_800161B8);
    {
      s32 arg5;
      s32 t0;
      s32 ix;
      void **pp;
      t0 = idx_1494[0];
      ix = idx_1494[1];
      pp = (void **)&D_800F19C0;
      t0 *= 4;
      t0 = (s32)((u8 *)tbl_125c + t0);
      ix <<= 2;
      arg5 = *(s32 *)(ix + (s32)tbl_125c);
      debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);
    }
    cdrom_ClearIrq();
    v0 = -1;
    goto check;
  }
  success:
  v0 = 0;
  idx_1495 = idx_1494 + 1;

  check:
  if (v0 != 0)
  {
    return -1;
  }

  new_var = 0xFF;
  if (sys_GetVblankCount() != 0)
  {
    saved = (*D_800A147C) & 3;
    poll:
    status = func_80080828();

    if (status != 0)
    {
      if (status & 4)
      {
        if (D_800A11B8 != 0)
        {
          ((void (*)(u8, void *)) D_800A11B8)(*idx_1495, &D_800F19A8);
        }
      }
      if (status & 2)
      {
        if (D_800A11B4 != 0)
        {
          ((void (*)(u8, void *)) D_800A11B4)(*idx_1494, &D_800F19A0);
        }
      }
      goto poll;
    }
    *D_800A147C = saved;
  }
  temp = (*idx_1494) & new_var;
  if (((temp == 2) || (temp == 5)) != 0)
  {
    *idx_1494 = 2;
    dst = a1;
    src = (u8 *) (&D_800F19A0);
    i = 7;
    if (a1 != 0)
    {
      do
      {
        b = *src;
        src++;
        i--;
        *dst = b;
        dst++;
      }
      while (i != (-1));
    }
    return temp;
  }
  if (a0 != 0)
  {
    return 0;
  }
  goto loop;
}
