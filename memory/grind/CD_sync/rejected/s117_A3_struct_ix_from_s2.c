//REPL:extern s32 D_800F19B8; => typedef struct { s32 timeout; s32 count; char *func; } CD_alarm; extern CD_alarm D_800F19B8;
//DROP:extern s32 D_800F19BC;
//DROP:extern void *D_800F19C0;
//REPL:extern void D_80016240; => extern char D_80016240[];
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
  D_800F19B8.timeout = VSync(-1) + 0x3C0;
  tbl_125c = D_800A125C;
  idx_1494 = &D_800A1494;
  idx_1495 = idx_1494 + 1;
  D_800F19B8.count = 0;
  D_800F19B8.func = D_80016240;
  loop:
  v0 = VSync(-1);

  if (D_800F19B8.timeout < v0)
  {
    puts(&D_800161B8);
    {
      s32 arg5;
      s32 t0;
      s32 ix;
      t0 = idx_1494[0];
      ix = idx_1494[1];
      t0 *= 4;
      t0 = (s32)((u8 *)tbl_125c + t0);
      ix <<= 2;
      arg5 = *(s32 *)(ix + (s32)tbl_125c);
      printf(&D_800161C8, D_800F19B8.func, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);
    }
    CD_flush();
    v0 = -1;
    goto check;
  }
  cnt = D_800F19B8.count;
  D_800F19B8.count = cnt - -1;
  if (0x3C0000 < cnt)
  {
    puts(&D_800161B8);
    {
      s32 arg5;
      s32 t0;
      s32 ix;
      t0 = idx_1494[0];
      ix = idx_1494[1];
      t0 *= 4;
      t0 = (s32)((u8 *)tbl_125c + t0);
      ix <<= 2;
      arg5 = *(s32 *)(ix + (s32)tbl_125c);
      printf(&D_800161C8, D_800F19B8.func, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);
    }
    CD_flush();
    v0 = -1;
    goto check;
  }
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
    saved = (*D_800A147C) & 3;
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
