/* s117 UPDATE (2026-09-04, rederive).  TWO changes to this file, both verified live.
 *
 * (1) CALL NAMES REPAIRED.  The banked body still used the pre-naming-wave names
 *     sys_VSync / tslTm2LoadImage_2 / debug_printf / cdrom_ClearIrq /
 *     sys_GetVblankCount / func_80080828, none of which exist in HEAD's
 *     src/system.c any more - so the banked candidate could not be spliced and
 *     compiled at all.  They are now VSync / puts / printf / CD_flush /
 *     CheckCallback / getintr (read off asm/funcs/CD_sync.s jal targets).
 *     Re-measured live this session: score 2 / build_insns 160 / target 160 /
 *     rules_dropped 0.  This is still the floor and still the SCALAR object
 *     model, so it splices against HEAD's declarations unchanged.
 *
 * (2) THE BETTER FORM IS NOW memory/grind/CD_sync/progress/s117-alarm-struct-nopp-2.c.
 *     Transplanting CD_datasync's s58 CD_alarm struct model (the owner directive's
 *     first probe) onto this body scores the SAME 2 / 160 while DELETING the
 *     `void **pp` pointer-alias FAKE outright.  It is link-identical to the
 *     scalar j1 chassis: the only object-level differences are five R_MIPS_LO16
 *     addends (D_800F19B8+4 / +8 vs D_800F19BC / D_800F19C0), which the linker
 *     resolves to the same addresses - verified instruction-for-instruction
 *     (tmp/grind/CD_sync/s117/dis_A0_j1.txt vs dis_A1_alarm_struct_nopp.txt,
 *     212 records, 5 diffs, all addend-only, identical registers throughout).
 *     On the SCALAR model dropping pp costs 6 points (s115_m1 = 9); under the
 *     struct model the member MEM (mem (const (plus (symbol_ref D_800F19B8) 8)))
 *     buys the early argument load for free, exactly as CD_datasync s58 measured.
 *     That form therefore carries ONE FAKE (the combine-foldable chain-extender
 *     for idx_1495) instead of two, and it removes the pp DECLARATION-PUN that
 *     the dispatch auto-scan flags on this file.  It is kept in progress/ rather
 *     than here because it needs a declaration surface (its //REPL / //DROP
 *     directives) and would not compile if spliced bare; the CD_datasync s58
 *     precedent files its struct form the same way.
 *
 * RESIDUAL AT 2, READ OFF THE BYTES (tmp/grind/CD_sync/s117/cmp.py): exactly the
 * index 54/55 transposition - target `addu $v0,$v0,$s3 ; sll $a0,$a0,2`, ours
 * `sll $a0,$a0,2 ; addu $v0,$v0,$s3`.  Every other one of the 160 matches,
 * registers included.  See hypotheses.md s117 for the QTY_CMP_PRI tie that this
 * session finally measured (the frontier's reference-count model was wrong).
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
  D_800F19B8 = VSync(-1) + 0x3C0;
  tbl_125c = D_800A125C;
  idx_1494 = &D_800A1494;
  idx_1495 = (u8 *)((u8 *)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C) + 1); /* FAKE: combine-foldable chain-extender (link-constant delta; folds to &D_800A1494 + 1 with ZERO emitted bytes, build_insns 160 == target), mechanism: flow.c records the extra reg_n_refs before combine.c folds the SYMBOL_REF difference (expr.c::expand_expr emits the subsi3/addsi3 pair, unfoldable in cse.c per s97); family: .claude/rules/dead-store-fake-exception.md:32-46 (owner ruling 2026-07-01); lever-exhaustion: memory/grind/CD_sync/hypotheses.md s96-s105 (~40 decompositions, 122 rejected forms); load-bearing: s105 measured all three honest respellings (idx_1494+1, &idx_1494[1], &D_800A1494+1) at masked=15 vs 2 */
  D_800F19BC = 0;
  D_800F19C0 = &D_80016240;
  loop:
  v0 = VSync(-1);

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
  puts(&D_800161B8);

  {
    s32 arg5;
    s32 t0;
    s32 ix;
    void **pp;
    t0 = idx_1494[0];
    ix = idx_1494[1]; /* s106: honest fresh local - the v0 staged-value borrow it replaces is NOT load-bearing (measured 2 == 2, 160/160) */
    pp = (void **)&D_800F19C0; /* FAKE: pointer-alias staging the D_800F19C0 load early; mechanism: local-alloc.c update_equiv_regs refs-2 sink defeat; lever-exhaustion in WIP history */
    t0 *= 4;
    t0 = (s32)((u8 *)tbl_125c + t0);
    ix <<= 2;
    arg5 = *(s32 *)(ix + (s32)tbl_125c);
    printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);
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
