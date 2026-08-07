/* REJECTED - best find of the session-14 SANDBOX-ALIGNED permuter campaign on
 * the const (RTX_UNCHANGING_P) chassis.  Permuter 26 against that chassis's
 * base 29 - the first genuine permuter-score DESCENT this function has ever
 * produced - and it screens sandbox 10 / 91 where the base it descended from
 * is 9 / 91.  Kept as the measurement that kills the permuter modality on
 * evidence rather than on session 13's (falsified) 60x-reordering model:
 * even with the objective re-weighted to REGALLOC 1 / REORDERING 2 /
 * INSERTION-DELETION 4, within-basin descent is still anti-correlated with
 * the position-locked sandbox, because difflib RE-ALIGNS the stream and our
 * whole residual is positional.
 */
s32 saEft01Init(s32 a0)
{
  s32 v0;
  s32 cnt;
  s32 *tbl_11dc;
  u8 *idx_1494;
  const s32 *tbl_125c;
  const s32 *new_var;
  D_800F19B8 = sys_VSync(-1) + 0x3C0;
  tbl_11dc = D_800A11DC;
  idx_1494 = &D_800A1494;
  tbl_125c = (const s32 *) D_800A125C;
  D_800F19BC = 0;
  D_800F19C0 = &D_800162C0;
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
  do
  {
    s32 arg4;
    new_var = tbl_125c;
    tslTm2LoadImage_2(&D_800161B8);
    v0 = new_var[idx_1494[0]];
    arg4 = v0;
    debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], arg4, new_var[idx_1494[1]]);
    cdrom_ClearIrq();
  }
  while (0);

  v0 = -1;
  goto check;
  success:
  v0 = 0;

  check:
  if (v0 != 0)
  {
    return -1;
  }

  if ((*D_800A14C0) & 0x1000000)
  {
    if (a0 == 0)
    {
      goto loop;
    }
    return 1;
  }
  return 0;
}
