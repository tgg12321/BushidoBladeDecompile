s32 CD_ready(s32 a0, u8 *a1)
{
  s32 v0;
  s32 cnt;
  u8 *idx_1494;
  u8 *idx_1495;
  u8 *idx_1496;
  s32 *tbl_125c;
  u8 saved;
  s32 status;
  u8 *src;
  u8 *dst;
  s32 i;
  u8 b;
  D_800F19B8 = VSync(-1) + 0x3C0;
  tbl_125c = D_800A125C;
  idx_1494 = &D_800A1494;
  idx_1495 = 1 + idx_1494;
  idx_1496 = idx_1494 + 2;
  D_800F19BC = 0;
  D_800F19C0 = &D_80016248;
  loop:
  v0 = VSync(-1);

  if (D_800F19B8 < v0)
  {
    goto do_timeout;
  }
  cnt = D_800F19BC;
  D_800F19BC = cnt + 1;
  if (!(0x3C0000 < cnt))
  {
    goto success;
  }
  do_timeout:
  puts(&D_800161B8);

  {
    s32 arg5;
    s32 arg4;
    ;
    arg4 = tbl_125c[idx_1494[0]];
    printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, tbl_125c[idx_1494[1]]);
  }
  CD_flush();
  v0 = -1;
  goto check;
  success:
  v0 = 0;

  check:
  if (v0 != 0)
  {
    return -1;
  }

  new_var = 0xFF;
  if (CheckCallback() != 0)
  {
    saved = (*D_800A147C_2) & 3;
    poll:
    status = getintr();

    if (status != 0)
    {
      if (status & 4)
      {
        if (D_800A11B8 != 0)
        {
          ((void (*)(u8, void *)) D_800A11B8)(*idx_1495, &D_800F19A8);
        }
        ;
      }
      if (status & 2)
      {
        if (D_800A11B4)
        {
          ((void (*)(u8, void *)) D_800A11B4)(*idx_1494, &D_800F19A0);
        }
      }
      goto poll;
    }
    *D_800A147C_2 = saved;
  }
  {
    register s32 check asm("$6");
    check = *idx_1496;
    if (!check) goto check2;
    *idx_1496 = 0;
    src = (u8 *) (&D_800F19B0);
    if (a1 != 0)
    {
      dst = a1;
      i = 7;
      do
      {
        b = *src;
        src++;
        i--;
        *dst = b;
        dst++;
      }
      while (i != (-1));
      return check;
    }
    goto done;
    check2:
    check = *(idx_1496 - 1);
    if (check)
    {
      *(idx_1496 - 1) = 0;
      dst = a1;
      src = (u8 *) (&D_800F19A8);
      if (dst != 0)
      {
        i = 7;
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
    }
    else
    {
      goto after_blocks;
    }
    done:
    return check;
    after_blocks:
    if (a0 != 0)
    {
      return 0;
    }
    goto loop;
  }
}
