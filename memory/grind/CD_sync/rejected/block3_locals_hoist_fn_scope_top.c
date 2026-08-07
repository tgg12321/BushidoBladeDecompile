s32 cpu_side_move_dir_4(s32 a0, u8 *a1)
{
  s32 arg5;
  s32 t0;
  void **pp;
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
  idx_1495 = (u8 *)((u8 *)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C) + 1);
  D_800F19BC = 0;
  D_800F19C0 = &D_80016240;
  loop:
  v0 = sys_VSync(-1);

  if (D_800F19B8 < v0)
  {
    goto do_timeout;
  }
  cnt = D_800F19BC;
  D_800F19BC = cnt - -1;
  if (!(0x3C0000 < cnt))
  {
    goto success;
  }
  do_timeout:
  tslTm2LoadImage_2(&D_800161B8);

  {
    t0 = idx_1494[0];
    v0 = idx_1494[1]; /* FAKE: index staged through the (dead-here) v0 var per staged-value-reused-variable (owner-sanctioned 2026-07-03); mechanism: sched.c adjust_priority/birthing_insn_p; prior v0 value dead (re-set below before any read); lever-exhaustion: twin session-4, mirrors marionation */
    pp = (void **)&D_800F19C0; /* FAKE: pointer-alias staging the D_800F19C0 load early; mechanism: local-alloc.c update_equiv_regs refs-2 sink defeat; lever-exhaustion in WIP history */
    t0 *= 4;
    t0 = (s32)((u8 *)tbl_125c + t0);
    v0 <<= 2; /* FAKE: continued staging - keeps the arg5 index-chain non-birthing so the sll places at the target slot */
    arg5 = *(s32 *)(v0 + (s32)tbl_125c);
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
