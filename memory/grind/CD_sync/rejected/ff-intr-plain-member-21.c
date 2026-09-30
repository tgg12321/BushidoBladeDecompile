s32 CD_sync(s32 a0, u8 *a1)
{
  s32 v0;
  s32 cnt;
  s32 *tbl_125c;
  u8 saved;
  s32 status;
  u8 *src;
  u8 *dst;
  s32 i;
  u8 b;
  s32 temp;
  D_800F19B8 = VSync(-1) + 0x3C0;
  tbl_125c = CD_intstr; /* FAKE: pointer alias (second handle) to the CD_intstr table per pointer-alias-fake-exception (owner ruling 2026-07-01, the `Type* t = &g_Thing;` redundant-second-handle shape), mechanism: global.c seats the base in $s3 across the whole function as the target does (asm/funcs/CD_sync.s:16-17); lever-exhaustion: s126 ablation C1 (direct D_800A125C[] subscript) = 31/160, plus the 125-session ledger in memory/grind/CD_sync/hypotheses.md */
  Alarm_plus_0x4 = 0;
  Alarm_plus_0x8 = &D_80016240;
  loop:
  v0 = VSync(-1);

  if (D_800F19B8 < v0)
  {
    goto do_timeout;
  }
  cnt = Alarm_plus_0x4;
  Alarm_plus_0x4 = cnt + 1;
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
    void **pp;
    t0 = Intr.sync; /* FAKE: named intermediate for the sync byte, placed BEFORE the wrap (loop depth 1), mechanism: flow.c:2081 loop_depth-weighted reg_n_refs feeds local-alloc.c:1660 qty_compare_1 - the depth-1 mention leaves the merged chain-A quantity below the second table read's priority, so the chain takes $a0 and the value $v1 exactly as at asm/funcs/CD_sync.s:49/56/60/65; lever-exhaustion: s126 probe B3 (read inlined into the src address) = 14/160, the 125-session t0 ledger in hypotheses.md s118-s125 */
    do { /* FAKE: do-while(0) wrap per do-while-zero-exception (owner ruling 2026-07-06), mechanism: sched.c:2081 loop-note barrier on the first insn inside (the ready-byte address chain) orders the sync-byte load ahead of it and every later register-argument load after it, and flow.c loop_depth ref weighting seats tbl_125c in $s3; lever-exhaustion: s126 ablation A8 (wrap removed) = 25/160 */
      pB = (s32 *)((Intr.ready << 2) + (s32)tbl_125c); /* FAKE: named address intermediate (fresh, once-written, once-read, real value = `addu $v0,$v0,$s3` at asm/funcs/CD_sync.s:56), mechanism: rank_for_schedule INSN_LUID tie-break (sched.c:2462) between the boosted chain-B address insn and the boosted chain-A shift - the address must precede the shift and the value load follow it in RTL order; lever-exhaustion: s126 probe B1 (folded back into the arg5 load) = 7/160 */
      src = (u8 *)((t0 << 2) + (s32)tbl_125c); /* FAKE: chain-A address staged through the (dead-here) src copy-loop variable per staged-value-reused-variable (owner-sanctioned 2026-07-03), mechanism: the multi-set destination keeps the addu unboosted (birthing_insn_p sched.c:2505) so it fills the backward-pass slot behind the sw instead of the shift, and global.c seats it in $a0 with src's copy-loop lives; lever-exhaustion: s126 ablation A6 (fresh local `ta` instead of the reused src) = 8/160 */
      arg5 = *pB; /* FAKE: named intermediate for the fifth (stack) argument (fresh, once-written, once-read, real value = `lw $v1,0($v0)` at asm/funcs/CD_sync.s:60), mechanism: calls.c store_one_arg - a named value is loaded before the call sequence and stored by the sw at the target slot 64; lever-exhaustion: s126 probe B2 (passed as *pB directly) = 9/160 */
      pp = &Alarm_plus_0x8; /* FAKE: pointer alias (second handle) to the alarm callback slot per pointer-alias-fake-exception (owner ruling 2026-07-01, the `Type* t = &g_Thing;` redundant-second-handle shape), mechanism: calls.c:1652-1664 expand_call precomputes a register argument whose rtx_cost > 2 into a pseudo inside a loop (preserve_subexpressions_p), whereas `*pp` is a cheap mem(reg) that stays in the call sequence and cse folds the alias back to the target's `lui $a1 / lw $a1` at asm/funcs/CD_sync.s:51-52; lever-exhaustion: s126 ablation A5 (direct D_800F19C0 read) = 18/160; the CD_alarm struct spelling that this candidate carried through s117-s125 is BANNED (decisions.md 2026-09-06 11:38) and is removed here */
      printf(&D_800161C8, *pp, CD_comstr[CD_com], *(s32 *)src, arg5);
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

  do { /* FAKE: do-while(0) wrap per do-while-zero-exception (owner ruling 2026-07-06 - sanctioned for ANY codegen effect incl. register allocation), mechanism: flow.c:2081 loop_depth-weighted reg_n_refs lifts the idx_1494 / idx_1495 / saved allocnos in global.c's priority sort so they take $s2/$s4/$s1 instead of $s1/$s6/$s2; single level is sufficient here (no nested wrap needed); lever-exhaustion: s126 ablation V1 (wrap removed) = 18/160, all 18 being callee-saved register substitutions on an otherwise order-exact 160/160 stream */
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
        if (CD_cbready != 0)
        {
          ((void (*)(u8, void *)) CD_cbready)(Intr.ready, &Result_plus_0x8);
        }
      }
      if (status & 2)
      {
        if (CD_cbsync != 0)
        {
          ((void (*)(u8, void *)) CD_cbsync)(Intr.sync, &Result);
        }
      }
    }
    }
    while (1);
    *D_800A147C = saved;
  }
  } while (0);
  temp = Intr.sync & 0xFF;
  if (((temp == 2) || (temp == 5)) != 0)
  {
    Intr.sync = 2;
    dst = a1;
    src = (u8 *) (&Result);
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
