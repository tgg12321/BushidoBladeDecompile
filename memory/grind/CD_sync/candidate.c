/* CD_sync candidate - s118 (2026-09-04).  Honest floor 2/160, build_insns 160,
 * rules_dropped 0, measured live this session on HEAD's src/system.c.
 *
 * THIS BODY REPLACES the s117 scalar/pp body that used to sit here.  It is the
 * pp-FREE CD_alarm-struct chassis (s117 hypothesis H117-1, CONFIRMED): the
 * per-word D_800F19BC / D_800F19C0 externs are dropped and D_800F19B8 is
 * declared as `typedef struct { s32 timeout; s32 count; char *func; } CD_alarm;`
 * so the printf argument is a member MEM.  Same 2/160 as the scalar body, but
 *   - ONE FAKE construct instead of two (the `void **pp` pointer alias is gone),
 *   - no declaration pun for the dispatch auto-scan to flag.
 * The five R_MIPS_LO16 addend differences the struct model produces are all
 * masked by engine/score.py and were verified addend-only against objdump
 * records in s117, so 2 is 2.
 *
 * The leading //REPL: / //DROP: lines are the declaration-surface directives
 * consumed by tmp/grind/CD_sync/s117/apply.py (region-scoped so CD_datasync's
 * duplicate declaration block is never touched).  They are also valid C
 * comments, so this file reads as the body plus a decl-surface manifest.
 *
 * RESIDUAL (unchanged since s115): a single transposition at target indices
 * 54/55 - target `addu $v0,$v0,$s3 ; sll $a0,$a0,2`, ours the other way round.
 * All 158 other instructions match including every register.  See
 * hypotheses.md s117/s118 for the local-alloc quantity table that governs it.
 */
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
  idx_1495 = (u8 *)((u8 *)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C) + 1); /* FAKE: combine-foldable chain-extender (link-constant delta; folds to &D_800A1494 + 1 with ZERO emitted bytes, build_insns 160 == target), mechanism: flow.c records the extra reg_n_refs before combine.c folds the SYMBOL_REF difference (expr.c::expand_expr emits the subsi3/addsi3 pair, unfoldable in cse.c per s97); family: .claude/rules/dead-store-fake-exception.md:32-46 (owner ruling 2026-07-01); lever-exhaustion: memory/grind/CD_sync/hypotheses.md s96-s105 (~40 decompositions, 122 rejected forms); load-bearing: s105 measured all three honest respellings (idx_1494+1, &idx_1494[1], &D_800A1494+1) at masked=15 vs 2 */
  D_800F19B8.count = 0;
  D_800F19B8.func = D_80016240;
  loop:
  v0 = VSync(-1);

  if (D_800F19B8.timeout < v0)
  {
    goto do_timeout;
  }
  cnt = D_800F19B8.count;
  D_800F19B8.count = cnt - -1;
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
    t0 = idx_1494[0];
    ix = idx_1494[1]; /* s106: honest fresh local - the v0 staged-value borrow it replaces is NOT load-bearing (measured 2 == 2, 160/160) */
    t0 *= 4;
    t0 = (s32)((u8 *)tbl_125c + t0);
    ix <<= 2;
    arg5 = *(s32 *)(ix + (s32)tbl_125c);
    printf(&D_800161C8, D_800F19B8.func, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);
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
