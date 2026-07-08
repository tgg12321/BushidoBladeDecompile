/* s36 v01 - masked 10 / 178. LITERAL saEft01Init-sibling transplant:
 * named arg5-first locals, NO pp alias, NO staging webs, direct globals.
 * KEY MECHANISM RESULT (not just a kill): this is the SIMPLEST spelling ever
 * measured that achieves the TARGET middle-addu pair order
 * (sll $3; addu $3,$3,$21; sll $2) - honest plain C, zero FAKE constructs in
 * the window. Residuals: seat trade (arg5 web v1-vs-v0, arg4 web v0-vs-a0,
 * the known coupled fixed point), lbu [1] stolen into the tslTm2 jal delay
 * slot (arg5-first tree order flips the T-16 launch, s15 mechanism), arg4's
 * addu+deref emitted early (target defers past arg5 deref + arg3 lbu).
 * ALSO PROVEN: D_800F19C0 passed directly lands the lw a1 in the TARGET slot
 * without the pp alias in this window shape - pp is INERT in the named-local
 * family (v02 arg4-first no-pp = 9 == s9v01 with-pp = 9; v01 10 vs s9v02 11).
 * The sibling saEft01Init (queue dist 18, 15 rules) has a byte-shape-IDENTICAL
 * do_timeout window vs its own target (same globals, same fmt string) and its
 * current C suffers the SAME seat trade - one shared unknown original spelling
 * decides both functions' windows. */
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
  do { /* FAKE: do-while(0) loop-note ref weighting seats tbl_125c in s5 (SOTN FAKE-class match device; do-while-zero-exception 2026-07-06) */
  tslTm2LoadImage_2(&D_800161B8);

  {
    s32 arg5;
    s32 arg4;
    arg5 = tbl_125c[idx_1494[1]];
    arg4 = tbl_125c[idx_1494[0]];
    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, arg5);
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

  new_var = 0xFF;  /* FAKE: opaque mask variables (with new_var3) keep the target's redundant `andi ,0xff` alive (named-local constant-holder family) */
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
    do { do { *idx_1496 = 0; } while (0); } while (0); /* FAKE: NESTED do-while(0) - double loop-note weighting lifts idx_1496's allocno priority to 1600, above arg1's 952. Single-level MEASURED insufficient 2026-07-06 */
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
