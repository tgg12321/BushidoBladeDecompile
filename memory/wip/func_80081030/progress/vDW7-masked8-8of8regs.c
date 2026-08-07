/* BREAKTHROUGH 2026-07-05 session-8: masked=8, ALL 8 callee-saved regs match target.
 * The 10-session REGISTER ROTATION is SOLVED. Mechanism:
 *   do{ body }while(0) wrapper emits NOTE_INSN_LOOP_BEG -> frequency-weights the body
 *   (fixes i1494/i1496/arg1/tbl/i1495 order); arg0 test lives OUTSIDE it in a goto
 *   back-edge (weight-1 -> arg0 lands dead-last at s7). LITERAL 0xFF/3 masks (NOT the
 *   new_var int -- that materializes to a saved reg under do-while(0)). Sanctioned
 *   printf staging + saved-stage kept.
 * REMAINING (masked 8, 3 insns short of 179): do_timeout sll/addu pair-swap (1) +
 *   two `andi rX,rY,0xff` after the check lbu's (GCC elides `&0xFF` on a proven byte;
 *   target keeps them -> 2 andi + 1 load-delay nop = the 3 missing insns).
 * HONESTY: do-while(0) here is used for its RA-weighting effect (copy blocks return,
 *   not break -> no break-semantic). Sanctioned as a construct (SOTN 18+, do-while-zero
 *   -exception) but this USE needs cheat-review / user ruling. Target RA appears ONLY
 *   reproducible with body-weight + arg0-outside == this structure, arguing faithful. */
s32 marionation_Exec(s32 a0, u8 *a1)
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
  do
  {
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
    pp = (void **)&D_800F19C0; /* FAKE: pointer-alias staging (staged-value-reused-variable / pointer-alias, owner-sanctioned 2026-07-03) - the D_800F19C0 load placed early so the a1 arg loads at the target slot */
    t0 *= 4;
    t0 = (s32)((u8 *)tbl_125c + t0);
    v0 = idx_1494[1]; /* FAKE: index staged through the (dead-here) v0 var per staged-value-reused-variable (owner-sanctioned 2026-07-03); v0's prior value is dead (re-set below before any read) */
    v0 <<= 2; /* FAKE: continued staging per staged-value-reused-variable */
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

  if (sys_GetVblankCount() != 0)
  {
    v0 = *D_800A147C_2; /* FAKE: byte staged through dead v0 (staged-value-reused-variable) */
    saved = v0 & 3;
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
    v0 = *idx_1496; /* FAKE: staged through dead v0 */
    check = v0 & 0xFF;
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
    v0 = *(idx_1496 - 1); /* FAKE: staged through dead v0 */
    check = v0 & 0xFF;
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
  }
  }
  while (0);
  v0 = 0;
  if (a0 != 0)
  {
    return v0;
  }
  goto loop;
}
