/* HONEST BASELINE (2026-07-05) - masked 30, m2c-tail structure, all THREE
 * unsanctioned register-web cheats stripped to natural C:
 *   idx_1495 = 1 + idx_1494;      (was cross-symbol tbl-derived FAKE)
 *   idx_1496 = idx_1494 + 2;      (was iq3 `+=1;+=1;` double-split)
 *   D_800F19C0 = &D_80016248;     (was idx_1494-rebase FAKE)
 * Kept: sanctioned printf staging (staged-value-reused-variable + pointer-
 * alias, owner-sanctioned 2026-07-03) + saved &= 3 split. Apply to
 * src/system.c, `sandbox marionation_Exec --disable all` must say 30.
 * This is the TRUE resume point; the committed build's masked-4 (mh5, now
 * rejected/mh5-mirage-masked4.c) was a register-masked mirage on the cheats.
 * The whole remaining gap is REGISTER ALLOCATION - see notes.md. */
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
  idx_1495 = 1 + idx_1494;
  idx_1496 = idx_1494 + 2;
  D_800F19BC = 0;
  D_800F19C0 = &D_80016248;
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
  do {
  tslTm2LoadImage_2(&D_800161B8);

  {
    s32 arg5;
    s32 t0;
    void **pp;
    t0 = idx_1494[0];
    pp = (void **)&D_800F19C0; /* FAKE: pointer-alias staging (staged-value-reused-variable / pointer-alias, owner-sanctioned 2026-07-03) - the D_800F19C0 load placed early so the a1 arg loads at the target slot */
    t0 *= 4;
    t0 = (s32)((u8 *)tbl_125c + t0);
    v0 = idx_1494[1]; /* FAKE: index staged through the (dead-here) v0 var per staged-value-reused-variable (owner-sanctioned 2026-07-03); v0's prior value is dead (re-set below before any read) */
    v0 <<= 2; /* FAKE: continued staging per staged-value-reused-variable */
    arg5 = *(s32 *)(v0 + (s32)tbl_125c);
    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);
  }
  cdrom_ClearIrq();
  } while (0);
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
  do {
  if (sys_GetVblankCount() != 0)
  {
    saved = *D_800A147C_2;
    saved &= 3;  /* split (y1): sanctioned split-init accumulation */
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
  } while (0);
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
    if (check != 0)
    {
      *(idx_1496 - 1) = 0;
      dst2 = a1;
      src = (u8 *) (&D_800F19A8);
      i = 7;
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
      done:
      return check;
    }
    v0 = 0;
    if (a0 != 0)
    {
      return v0;
    }
    goto loop;
  }
}
