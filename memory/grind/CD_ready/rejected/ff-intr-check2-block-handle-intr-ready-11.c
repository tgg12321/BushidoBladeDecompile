s32 CD_ready(s32 a0, u8 *a1)
{
  s32 v0;
  s32 cnt;
  volatile CD_intr *intr;
  volatile u8 *idx_1495;
  volatile u8 *idx_1496;
  int new_var;
  int new_var3;
  s32 *tbl_125c;
  u8 saved;
  s32 status;
  u8 *src;
  u8 *dst;
  u8 *dst2;
  s32 i;
  D_800F19B8 = VSync(-1) + 0x3C0;
  tbl_125c = CD_intstr; /* FAKE: pointer alias to the CD_intstr table per pointer-alias-fake-exception, mechanism: the base is held in s5 across the poll loop as in the target; lever-exhaustion: direct-subscript spellings s66 s07 (14) */
  intr = &Intr; /* FAKE: pointer alias (second handle) to the one libcd Intr object per pointer-alias-fake-exception (owner ruling 2026-07-01, the `Type* t = &g_Thing;` redundant-second-handle shape; Intr is Sony's `static volatile CD_intr`, owner ruling Q42), mechanism: the base is held in s2 across the poll loop as in the target; lever-exhaustion (ff-intr 2026-09-30, memory/grind/CD_ready/evidence.md): plain Intr member access with no handles = 31/179; Intr.sync in place of the handle at one site = 5 (t0), 20 (sync callback); type-level direct spellings s60 (4/180), s88 VD (2/180) */
  idx_1495 = &Intr.ready; /* FAKE: second handle to the ready byte of the same Intr object per pointer-alias-fake-exception (a member address), mechanism: base register s6 for the ready byte in the callback block as in the target; lever-exhaustion (ff-intr 2026-09-30): Intr.ready at the ready callback = 24/179, intr->ready there = 13/179; s60 */
  idx_1496 = &Intr.c; /* FAKE: third handle, to the completion byte of the same Intr object, per pointer-alias-fake-exception (a member address), mechanism: base register s3 (`addiu s3,s2,2`) for the completion byte as in the target; `*(idx_1496 - 1)` in check2 is the ready byte reached through it, arithmetic inside the one object, and that volatile store is load-bearing through reorg.c:760 (resource_conflicts_p: a volatile store sets set.volatil, so fill_simple_delay_slots takes nothing into the `beqz a2` slot - the target's nop); lever-exhaustion (ff-intr 2026-09-30): Intr.c at the check / clear = 15 / 14, intr->c at both = 19, the ready byte in check2 spelled Intr.ready = 15 (check) / 14 (clear), *idx_1495 = 19 / 19, intr->ready at both = 13; s88 R1 (non-volatile idx_1496: 2/178, reorg hoists `move a1,s4` into the slot) */
  Alarm_plus_0x4 = 0;
  Alarm_plus_0x8 = &D_80016248;
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
    s32 tb;
    void **pp;
    t0 = intr->sync; /* FAKE: sync-byte read placed before the wrap (loop depth 1), mechanism: flow.c loop_depth-weighted reg_n_refs feeds local-alloc.c:1660 qty_compare - the depth-1 mention (qty refs 7, not 8) puts the merged chain-A quantity below the arg5 value's priority, so the value takes $v1 and the chain $a0; lever-exhaustion: s66 s01-s08, s88 N1/V2n/G2 (chain inside the wrap, refs 12: score 15) - hypotheses.md s88 */
  do { /* FAKE: do-while(0) wrap, mechanism: sched.c:2081 loop-note barrier on the first insn inside (the ready-byte load) orders the sync-byte load ahead of it and every later register-argument load after it, and flow.c loop_depth ref weighting seats tbl_125c in s5 (SOTN FAKE-class match device; do-while-zero-exception 2026-07-06); lever-exhaustion: wrap ablated s81 2 -> 12, s88 H4 0 -> 30 */
    tb = intr->ready; /* FAKE: named intermediate for the ready byte (fresh, once-written, once-read, real value = lbu v0,1(s2)), mechanism: expand argument staging - with arg5 it keeps the stack argument's chain out of the call sequence; lever-exhaustion: s88 H6 (tb and arg5 inlined into the printf call) = 9 */
    pB = (s32 *)((tb << 2) + (s32)tbl_125c); /* FAKE: named address intermediate (fresh, once-written, once-read, real value = addu v0,v0,s5), mechanism: rank_for_schedule INSN_LUID tie-break (sched.c:2462) between the boosted chain-B address insn and the boosted chain-A shift - the address must precede the shift and the value load follow it in RTL order; lever-exhaustion: s88 H5 (folded back into the value load) = 2 = the s80-s87 floor */
    src = (u8 *)((t0 << 2) + (s32)tbl_125c); /* FAKE: chain-A address staged through the (dead-here) src var per staged-value-reused-variable (owner-sanctioned 2026-07-03), mechanism: the multi-set destination keeps the addu unboosted (birthing_insn_p sched.c:2505) so it fills the backward-pass slot behind the sw instead of the shift, and global.c seats it in $a0 with src's copy-loop lives; lever-exhaustion: s87 F4/F5 (fresh ta: 9), s88 N1/V2n/G2 (in-place t0: 15), s87 P5a/P5b (5/9) */
    arg5 = *pB; /* FAKE: named intermediate for the fifth (stack) argument (fresh, once-written, once-read, real value = lw v1,0(v0)), mechanism: calls.c store_one_arg - a named value is loaded before the call sequence and stored by the sw at the target slot; lever-exhaustion: s88 H6 (passed as *pB directly) = 9 */
    pp = &Alarm_plus_0x8; /* FAKE: pointer alias (second handle) to the alarm callback slot per pointer-alias-fake-exception (owner ruling 2026-07-01, the `Type* t = &g_Thing;` redundant-second-handle shape), mechanism: calls.c:1652-1664 expand_call precomputes a register argument whose rtx_cost > 2 into a pseudo inside a loop (preserve_subexpressions_p) - the bare `mem(symbol_ref D_800F19C0)` is copied to a pseudo before the chain-A/chain-B insns and the sw (F1.combine insn 119 + `move a1`), whereas `*pp` is a cheap `mem(reg)` that stays in the call sequence and cse folds the alias back to `(set a1 (mem (symbol_ref D_800F19C0)))` (F2f.combine insn 136), the target's `lui a1/lw a1` at slots 53-54; lever-exhaustion: direct global read s88d F1 = 23/180 (a1 load displaced to slots 61-62, seats shuffled), s53-s57 scalar model = 9, the CD_alarm struct spelling is BANNED (decisions.md 2026-09-06 11:38); placement inside the block is byte-inert (s88d F2a/F2b/F2c/F2e/F2f all 0/179), function-scope placement F2d = 10/183 (pp becomes a loop-carried callee-saved live range) */
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

  new_var = 0xFF;  /* FAKE: opaque mask variables (with new_var3) keep the target's redundant `andi ,0xff` alive (named-local constant-holder family). Alternatives exhausted and recorded in memory/wip/CD_ready/notes.md: u8-typed checks fold via PROMOTE_MODE+combine (measured 17), staged raw byte folds (proven byte); the symbolic mask is the one spelling combine cannot fold */
  new_var3 = 0xFF;
  do { /* FAKE: do-while(0) loop-note ref weighting seats intr/idx_1495 in s2/s6 */
  if (CheckCallback() != 0)
  {
    saved = *D_800A147C & 3;
    do
    {
    status = getintr();

    if (status == 0) break;
    {
      if (status & 4)
      {
        if (CD_cbready != 0)
        {
          ((void (*)(u8, void *)) CD_cbready)(*idx_1495, &Result_plus_0x8);
        }
        ;
      }
      if (status & 2)
      {
        if (CD_cbsync)
        {
          ((void (*)(u8, void *)) CD_cbsync)(intr->sync, &Result);
        }
      }
    }
    }
    while (1);
    *D_800A147C = saved;
  }
  } while (0);
  {
    s32 check;
    volatile u8 *rdy;
    check = *idx_1496 & new_var;
    if (!check) goto check2;
    do { do { *idx_1496 = 0; } while (0); } while (0); /* FAKE: NESTED do-while(0) - double loop-note weighting lifts idx_1496's allocno priority to 1600, above arg1's 952. Single-level MEASURED insufficient 2026-07-06: i1496 pri 933 < arg1 952, i1496 falls s3->s4 (probe ledger, masked 4->14). Justification per do-while-zero-exception prerequisite 3 */
    src = (u8 *) (&Result_plus_0x10);
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
    }
    return check;
    check2:
    rdy = &intr->ready;
    check = *rdy & new_var3;
    if (!check) goto tail;
    do { *rdy = 0; } while (0); /* FAKE: do-while(0) loop-note weighting balances the check2 clear against check1's nested wrap */
    dst2 = a1;
    src = (u8 *) (&Result_plus_0x8);
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
    return check;
    tail:
    if (a0 == 0)
    {
      goto loop;
    }
    return 0;
  }
}
