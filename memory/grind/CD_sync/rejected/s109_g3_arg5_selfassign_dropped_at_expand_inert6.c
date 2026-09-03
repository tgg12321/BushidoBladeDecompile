/* REJECTED (s108, forensics):
   s108 p1: order-perfect (g3) but scores 6 - a pure a0/v1 exchange on 6 insns.
   local-alloc blk=3: qty(t0-shift p108) birth18 death24 refs2 -> pri 3333 TIES
   qty(arg5-value p100) birth20 death26 refs2 -> pri 3333; qty_compare_1's
   `*q1 - *q2` index tie-break gives the t0 shift v1 and the arg5 value a0,
   the exact inverse of the target.
*/
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
  idx_1495 = (u8 *)((u8 *)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C) + 1); /* FAKE: combine-foldable chain-extender (link-constant delta; folds to &D_800A1494 + 1 with ZERO emitted bytes, build_insns 160 == target), mechanism: flow.c records the extra reg_n_refs before combine.c folds the SYMBOL_REF difference (expr.c::expand_expr emits the subsi3/addsi3 pair, unfoldable in cse.c per s97); family: .claude/rules/dead-store-fake-exception.md:32-46 (owner ruling 2026-07-01); lever-exhaustion: memory/grind/CD_sync/hypotheses.md s96-s105 (~40 decompositions, 122 rejected forms); load-bearing: s105 measured all three honest respellings (idx_1494+1, &idx_1494[1], &D_800A1494+1) at masked=15 vs 2 */
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
    s32 arg5;
    s32 t0;
    s32 ix;
    void **pp;
    t0 = idx_1494[0];
    ix = idx_1494[1];
    pp = (void **)&D_800F19C0;
    ix <<= 2;
    arg5 = *(s32 *)(ix + (s32)tbl_125c);
    arg5 = arg5; /* FAKE: same-value re-store of the arg5 value, mechanism: flow.c reg_n_refs (flow.c:1281/2081) counted before combine.c copy-propagation, lifting qty_n_refs at local-alloc.c:297, lever-exhaustion: memory/grind/CD_sync/hypotheses.md s96-s108 */
    t0 *= 4;
    t0 = (s32)((u8 *)tbl_125c + t0);
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
