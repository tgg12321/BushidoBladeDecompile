s32 marionation_Exec(s32 a0, u8 *a1)
{
  s32 v0;
  s32 cnt;
  u8 *idx_1494;
  u8 *idx_1495;
  u8 *idx_1496;
  int new_var;
  int new_var3;
  s32 *tbl_125c;
  u8 saved;
  s32 status;
  u8 *src;
  u8 *dst;
  u8 *dst2;
  s32 i;
  D_800F19B8 = sys_VSync(-1) + 0x3C0;
  tbl_125c = D_800A125C;
  idx_1494 = &D_800A1494;
  idx_1495 = (u8 *)((u8 *)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C) + 1); /* FAKE: pointer derived from the loaded tbl base instead of &D_800A1495; mechanism: cse.c symbol-fold defeat keeps the reg-relative address (target bytes have the addu chain); lever-exhaustion: direct &-forms fold to lui/addiu, prior sessions (git) */
  idx_1496 = idx_1494 + 2;
  D_800F19BC = 0;
  D_800F19C0 = (void *)((u8 *)idx_1496 + ((s32)&D_80016248 - ((s32)&D_800A1494 + 2))); /* FAKE: same reg-relative address family as idx_1495 (cse.c symbol-fold defeat, target-byte-pinned); lever-exhaustion in git history */
  loop:
  v0 = sys_VSync(-1);

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
  tslTm2LoadImage_2(&D_800161B8);

  {
    s32 arg5;
    s32 t0;
    void **pp;
    t0 = idx_1494[0];
    pp = (void **)&D_800F19C0; /* FAKE: pointer-alias staging the D_800F19C0 load early; mechanism: local-alloc.c update_equiv_regs refs-2 sink defeat so the a1 arg loads at target slot 53-54; lever-exhaustion: direct arg forms sink the load (measured, git history) */
    t0 *= 4;
    t0 = (s32)((u8 *)tbl_125c + t0);
    arg5 = tbl_125c[idx_1494[1]];
    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);
  }
  cdrom_ClearIrq();
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
  new_var3 = 0xFF;
  if (sys_GetVblankCount() != 0)
  {
    saved = (*D_800A147C_2) & 3;
    do
    {
    status = func_80080828();

    if (status == 0) break;
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
    }
    }
    while (1);
    *D_800A147C_2 = saved;
  }
  {
    s32 check;
    check = *idx_1496 & new_var;
    if (!check) goto check2;
    *idx_1496 = 0;
    src = (u8 *) (&D_800F19B0);
    dst = a1;
    if (a1 != 0)
    {
      i = 7;
      do
      {
        u8 bb;
        bb = *src;
        src++;
        i--;
        *dst = bb;
        dst++;
      }
      while (i != (-1));
      return check;
    }
    goto done;
    check2:
    check = *(idx_1496 - 1) & new_var3;
    if (check)
    {
      *(idx_1496 - 1) = 0;
      src = (u8 *) (&D_800F19A8);
      i = 7;
      dst2 = a1;
      if (dst2 != 0)
      {
        do
        {
          u8 bb;
          bb = *src;
          src++;
          i--;
          *dst2 = bb;
          dst2++;
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
