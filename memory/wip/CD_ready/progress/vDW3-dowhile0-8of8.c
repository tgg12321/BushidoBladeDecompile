s32 marionation_Exec(s32 arg0, u8 *arg1)
{
  s32 v0_2;
  s32 cnt;
  u8 *idx_1494;
  u8 *idx_1495;
  u8 *idx_1496;
  s32 *tbl_125c;
  u8 saved;
  s32 status;
  s32 a2;
  u8 *src;
  u8 *dst;
  u8 *dst2;
  s32 i;
  idx_1494 = &D_800A1494;
  idx_1495 = idx_1494 + 1;
  idx_1496 = idx_1494 + 2;
  tbl_125c = D_800A125C;
  D_800F19B8 = sys_VSync(-1) + 0x3C0;
  D_800F19BC = 0;
  D_800F19C0 = &D_80016248;
  loop:
  do
  {
    if ((D_800F19B8 < sys_VSync(-1)) || (cnt = D_800F19BC, D_800F19BC = cnt + 1, cnt > 0x3C0000))
    {
      tslTm2LoadImage_2(&D_800161B8);
      debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], tbl_125c[idx_1494[0]], tbl_125c[idx_1494[1]]);
      cdrom_ClearIrq();
      v0_2 = -1;
    }
    else
    {
      v0_2 = 0;
    }
    if (v0_2 != 0)
    {
      return -1;
    }
    if (sys_GetVblankCount() != 0)
    {
      saved = *D_800A147C_2 & 3;
      do
      {
        status = func_80080828();
        if (status == 0) break;
        if (status & 4)
        {
          if (D_800A11B8 != 0)
          {
            ((void (*)(u8, void *)) D_800A11B8)(*idx_1495, &D_800F19A8);
          }
        }
        if ((status & 2) && D_800A11B4 != 0)
        {
          ((void (*)(u8, void *)) D_800A11B4)(*idx_1494, &D_800F19A0);
        }
      }
      while (1);
      *D_800A147C_2 = saved;
    }
    a2 = *idx_1496 & 0xFF;
    if (a2 != 0)
    {
      *idx_1496 = 0;
      src = (u8 *) (&D_800F19B0);
      dst = arg1;
      if (arg1 != 0)
      {
        i = 7;
        do
        {
          *dst = *src;
          src++;
          dst++;
          i--;
        }
        while (i != (-1));
      }
      return a2;
    }
    a2 = *(idx_1496 - 1) & 0xFF;
    if (a2 != 0)
    {
      *(idx_1496 - 1) = 0;
      dst2 = arg1;
      src = (u8 *) (&D_800F19A8);
      if (dst2 != 0)
      {
        i = 7;
        do
        {
          *dst2 = *src;
          src++;
          dst2++;
          i--;
        }
        while (i != (-1));
      }
      return a2;
    }
  }
  while (0);
  if (arg0 != 0)
  {
    return 0;
  }
  goto loop;
}
