/* marionation_Exec CLOSER candidate P6 (2026-07-09) - masked 2, build 179 == target 179.
 * NEW FLOOR (prior vT40 = masked 4 at 178/179 for 57 grind sessions).
 * Chassis: the vT40 candidate with ONE change - idx_1496 declared `volatile u8 *`.
 * Identity basis: this function IS PsyQ libcd bios.c v1.86 CD_ready();
 * D_800A1494/95/96 are libcd's `static volatile CD_intr Intr {sync,ready,c}` -
 * volatility is ORIGINAL SEMANTICS (IRQ-mutated via InterruptCallback(2,callback)),
 * see memory/closer/libcd-identity.md. The volatile access to Intr.c/(ready via -1)
 * legitimately blocks reorg.c fill_simple_delay_slots at the check2 beqz -> the
 * region-3 delay-slot nop matches target for the first time.
 * NOTE this P6 spelling is PARTIAL volatile (only idx_1496); the honest-complete
 * form is all three pointers volatile (P5 = masked 8, window scheduling regresses).
 * Judge question pending: partial-volatile split view of one volatile object.
 * Residual (masked 2 + 2 unmasked branch-immediates):
 *   1. {sll a0 <-> addu v0} single transposition in the do_timeout window
 *      (same shape as csmd4's h5 residual - twins now share ONE micro-residual).
 *   2. both `beqz (a1==0)` guards branch to the FIRST copy of the identical
 *      `j;move v0,a2` tail (0x23c); target branches to the SECOND (0x28c).
 *      Known technique: cross-jump-store-tail-merge label-direction steering.
 * Requires decl earlier in TU: typedef struct { u8 sync; u8 ready; u8 c; } CD_intr;
 * extern volatile CD_intr D_800A1494;  (see src edits of this session)
 */
s32 marionation_Exec(s32 a0, u8 *a1)
{
  s32 v0;
  s32 cnt;
  u8 *idx_1494;
  u8 *idx_1495;
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
  D_800F19B8 = sys_VSync(-1) + 0x3C0;
  tbl_125c = D_800A125C;
  idx_1494 = (u8 *)&D_800A1494;
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
  do { /* FAKE: do-while(0) loop-note ref weighting seats tbl_125c in s5 (SOTN FAKE-class match device; do-while-zero-exception 2026-07-06) */
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

  new_var = 0xFF;  /* FAKE: opaque mask variables (with new_var3) keep the target's redundant `andi ,0xff` alive (named-local constant-holder family). Alternatives exhausted and recorded in memory/wip/marionation_Exec/notes.md: u8-typed checks fold via PROMOTE_MODE+combine (measured 17), staged raw byte folds (proven byte); the symbolic mask is the one spelling combine cannot fold */
  new_var3 = 0xFF;
  do { /* FAKE: do-while(0) loop-note ref weighting seats idx_1494/idx_1495 in s2/s6 */
  if (sys_GetVblankCount() != 0)
  {
    saved = *D_800A147C_2 & 3;
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
    do { do { *idx_1496 = 0; } while (0); } while (0); /* FAKE: NESTED do-while(0) - double loop-note weighting lifts idx_1496's allocno priority to 1600, above arg1's 952. Single-level MEASURED insufficient 2026-07-06: i1496 pri 933 < arg1 952, i1496 falls s3->s4 (probe ledger, masked 4->14). Justification per do-while-zero-exception prerequisite 3 */
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
    if (!check) goto tail;
    do { *(idx_1496 - 1) = 0; } while (0); /* FAKE: do-while(0) loop-note weighting balances the check2 clear against check1's nested wrap */
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
    do { /* FAKE: do-while(0) places NOTE_INSN_LOOP_BEG before the interior label so reorg.c mostly_true_jump predicts the check2 beqz taken */
    tail:
    if (a0 == 0)
    {
      goto loop;
    }
    } while (0);
    return 0;
  }
}
