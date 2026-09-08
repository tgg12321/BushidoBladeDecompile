s32 CD_sync(s32 a0, u8 *a1)
{
  int new_var;
  s32 v0;
  s32 cnt;
  volatile u8 *idx_1494;
  volatile u8 *idx_1495;
  s32 *tbl_125c;
  u8 saved;
  s32 status;
  u8 *src;
  u8 *dst;
  s32 i;
  u8 b;
  s32 temp;
  D_800F19B8 = VSync(-1) + 0x3C0;
  tbl_125c = D_800A125C; /* FAKE: pointer alias to the CD_intstr table per pointer-alias-fake-exception (owner ruling 2026-07-01), mechanism: global.c seats the base in $s3 across the poll loop as the target does (asm/funcs/CD_sync.s:16-17); lever-exhaustion: hypotheses.md s96-s125 */
  idx_1494 = &g_cd_status_a; /* FAKE: pointer alias to the libcd Intr status block per pointer-alias-fake-exception (owner ruling 2026-07-01); volatile comes from the TU declaration `extern volatile u8 g_cd_status_a;`; mechanism: global.c seats the base in $s2 (asm/funcs/CD_sync.s:18-19); lever-exhaustion: hypotheses.md s96-s125 */
  idx_1495 = 1 + idx_1494; /* FAKE: second handle (+1) per pointer-alias-fake-exception, mechanism: base register $s4 (`addiu s4,s2,1`, asm/funcs/CD_sync.s:20); lever-exhaustion: s105 (three honest respellings measured 15 vs 2) */
  D_800F19BC = 0;
  D_800F19C0 = &D_80016240;
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
    s32 t0;
    s32 *pB;
    s32 tb;
    void **pp;
    t0 = idx_1494[0]; /* FAKE: sync-byte read placed before the wrap (loop depth 1), mechanism: flow.c loop_depth-weighted reg_n_refs feeds local-alloc.c:1660 qty_compare; lever-exhaustion: hypotheses.md s118-s125 */
    do { /* FAKE: do-while(0) wrap per do-while-zero-exception (owner ruling 2026-07-06), mechanism: sched.c:2081 loop-note barrier orders the sync-byte load ahead of the ready-byte load; lever-exhaustion: CD_ready s81/s88 */
      tb = idx_1494[1]; /* FAKE: named intermediate for the ready byte (fresh, once-written, once-read, real value = lbu v0,1(s2)), mechanism: expand argument staging; lever-exhaustion: CD_ready s88 H6 */
      pB = (s32 *)((tb << 2) + (s32)tbl_125c); /* FAKE: named address intermediate (real value = addu v0,v0,s3), mechanism: rank_for_schedule INSN_LUID tie-break (sched.c:2462); lever-exhaustion: CD_ready s88 H5 */
      src = (u8 *)((t0 << 2) + (s32)tbl_125c); /* FAKE: chain-A address staged through the (dead-here) src var per staged-value-reused-variable (owner-sanctioned 2026-07-03), mechanism: birthing_insn_p sched.c:2505; lever-exhaustion: CD_ready s87 F4/F5 */
      arg5 = *pB; /* FAKE: named intermediate for the fifth (stack) argument, mechanism: calls.c store_one_arg; lever-exhaustion: CD_ready s88 H6 */
      pp = &D_800F19C0; /* FAKE: pointer alias (second handle) to the alarm callback slot per pointer-alias-fake-exception (owner ruling 2026-07-01), mechanism: calls.c:1652-1664 expand_call precomputes a costly register argument into a pseudo; lever-exhaustion: CD_ready s88d F1 = 23/180 */
      printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)src, arg5);
      CD_flush();
    } while (0);
  }
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
  do { /* FAKE: do-while(0) wrap per do-while-zero-exception (owner ruling 2026-07-06), mechanism: flow.c loop_depth-weighted reg_n_refs lifts the idx_1494/idx_1495 allocnos in global.c's priority sort; lever-exhaustion: CD_ready s88 (same wrap, same window) */
  if (CheckCallback() != 0)
  {
    saved = (*D_800A147C) & 3;
    do
    {
    status = getintr();

    if (status == 0) break;
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
    }
    }
    while (1);
    *D_800A147C = saved;
  }
  } while (0);
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
