/* s67 variant z08_empty_before_a5ld: EMPTY do{}while(0) before ALL[8] - the ledger's named note-innocence probe */
/* CD_ready candidate - s60 (2026-09-01). HONEST FLOOR: masked 2, build 179 == target 179,
 * rules_dropped 0. This SUPERSEDES the 59-session vT40 body (masked 4, build 178), which is
 * retained beside this file as candidate-vT40-masked4-no-volatile.c.
 *
 * PROVENANCE. This is the Closer-phase form `marionation_vAT1_notailwrap.c`, recovered with
 *   git show 043e4b80^:memory/closer/candidates/marionation_vAT1_notailwrap.c
 * It was banked 2026-07-09 at masked 2 and then set aside on a premise the owner corrected in
 * the 2026-09-01 FORECLOSED-BUCKET REVIEW (decisions.md:17795, Ruling A row `CD_ready (d4)`):
 * "re-score the banked vAT1 form post-`-mel`". Executed this session; it re-scores at 2 on the
 * current post-`-mel`, post-naming-wave chassis. Floor 4 -> 2.
 *
 * NAMING MAP (this body predates the naming wave). To splice into the current src/system.c:
 *   marionation_Exec->CD_ready, sys_VSync->VSync, tslTm2LoadImage_2->puts, debug_printf->printf,
 *   cdrom_ClearIrq->CD_flush, sys_GetVblankCount->CheckCallback, func_80080828->getintr.
 * Harness: tmp/grind/CD_ready/s60/splice.py (marker: INCLUDE_ASM("asm/funcs", CD_ready); at
 * src/system.c:379). All needed externs already exist at src/system.c:353-375.
 *
 * THE ONE CONSTRUCT THAT DISTINGUISHES THIS FROM vT40 - and the open family question.
 * `volatile u8 *idx_1496;` (the pointer to the IRQ-set CD status byte 0x800A1496 = g_cd_status_c).
 * It is LOAD-BEARING and MEASURED so: strip the qualifier and nothing else (s60 v01) and the score
 * goes 2 -> 4 with build_insns 179 -> 178, i.e. GCC hoists the flag load out of the `goto loop`
 * polling loop and the target's 179th instruction disappears. The target therefore contains an
 * access GCC only emits when the object is volatile.
 *   PRONG 1 of .claude/rules/legitimate-volatile-interrupt-touched.md is SATISFIED WITH CITATIONS:
 *     - IRQ writer: `getintr` writes the byte - asm/funcs/getintr.s:304 `sb $v0, %lo(D_800A1496)($at)`
 *       (also :244 for 1494, :274/:309 for 1495); func_800819C4 likewise at asm/funcs/func_800819C4.s:71.
 *     - Installed handler: `getintr()` is called from `cdrom_IrqHandler` (src/system.c:770), whose
 *       entry carries glabel D_80081F1C (src/system.c:758-768) and which is installed as the IRQ-2
 *       (CD-ROM) callback by `InterruptCallback(2, &D_80081F1C);` at src/system.c:609 and :630.
 *   IN-TU PRECEDENT (matched, committed, byte-correct code declares these very bytes volatile):
 *     - src/system.c:549-551 `extern volatile u8 g_cd_status_a; ... _b; ... _c;`
 *     - src/system.c:748-749 the same, inside cdrom_IrqHandler's own declaration block
 *     - src/system.c:770-771 `volatile u8 *s1 = &g_cd_status_b; volatile u8 *s3 = s1 - 1;`
 *       i.e. a matched function derives a volatile byte pointer BY POINTER ARITHMETIC, the same
 *       shape used here (`idx_1496 = idx_1494 + 2`).
 *   PRONG 2 (use-site shape) IS THE OPEN QUESTION and must be settled before any candidate-ready:
 *   the rule's exact list is spin-wait / double-read-across-sequence-point / IRQ-mutated-loop-bound.
 *   This use-site is the `loop:` ... `tail: if (a0 == 0) goto loop;` poll whose body calls VSync()
 *   and getintr() between successive reads of `*idx_1496` - closest to
 *   double-read-across-sequence-point, but it is not verbatim any of the three. Also note
 *   D_800A1494/95/96 carry NO entry in volatile_extern_allowlist.txt, so a candidate needs either
 *   that grant or a ruling that the already-shipped in-TU declarations cover it. DO NOT submit
 *   candidate-ready on this body without resolving that; the honest move is a ruling-request.
 *
 * MEASURED-DEAD RESPELLINGS OF THE SAME VOLATILE (all s60, banked in rejected/):
 *   - type-level on the global, pointer taken directly (`extern volatile u8 g_cd_status_c;
 *     idx_1496 = &g_cd_status_c;`): score 4, build 180 - the separate lui/%lo materialisation
 *     destroys the single-base addressing (identical finding to CD_sync s107, decisions.md:18333).
 *   - all three Intr pointers volatile (idx_1494/1495/1496): score 8, build 179.
 *   - same, with the base taken as `&g_cd_status_a` in cdrom_IrqHandler's exact shape: score 8.
 *   Only the 1496 access path may be volatile; 1494/1495 must stay plain.
 *
 * REMAINING RESIDUAL (masked 2, 179/179) - CORRECTED AND FULLY LOCALISED IN s61 (2026-09-01).
 * The s60 header guessed a register mismatch here; the s61 disassembly (tmp/grind/CD_ready/s61/
 * show.py) proves otherwise. EVERY register in all 179 instructions already matches the target,
 * both seats included. The entire residual is a transposition of two independent ALU insns in the
 * do_timeout printf-argument block:
 *     idx  BUILD              TARGET
 *      55  sll  $v0,$v0,2     sll  $v0,$v0,2
 *      56  sll  $a0,$a0,2     addu $v0,$v0,$s5
 *      57  addu $v0,$v0,$s5   sll  $a0,$a0,2
 *      58  lw   $v1,0($v0)    lw   $v1,0($v0)
 * Attribution READ from tmp/grind/CD_ready/dumps/system.sched2: sched2 insns 106 (sll a0) and
 * 120 (addu v0) tie on INSN_PRIORITY and rank_for_schedule falls through to INSN_LUID, so C
 * statement order decides. Moving `t0 *= 4` after the arg5 chain DOES produce the target order
 * (s61 w04) but inverts the local-alloc seat via qty_compare_1 - order and seat are each
 * independently reachable and anti-correlated through that one lever. The arithmetic of the
 * inequality that would satisfy both is written out at the end of hypotheses.md (s61).
 * Every t0-web / arg5 axis killed in s53-s59 was killed against the vT40 (masked-4) base; s61
 * re-ran the natural-C, source-position, decl-order, pseudo-split and variable-reuse families
 * against THIS base (all banked in evidence.md s61 and rejected/s61-*).
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
  {
    s32 arg5;
    s32 a5a;
    s32 t0;
    void **pp;
  do { /* FAKE: do-while(0) loop-note ref weighting seats tbl_125c in s5 (SOTN FAKE-class match device; do-while-zero-exception 2026-07-06) */
  tslTm2LoadImage_2(&D_800161B8);
    t0 = idx_1494[0];
    pp = (void **)&D_800F19C0;
    v0 = idx_1494[1];
    v0 <<= 2;
    a5a = v0 + (s32)tbl_125c;
    t0 *= 4;
    t0 = (s32)((u8 *)tbl_125c + t0);
  do { } while (0);
    arg5 = *(s32 *)a5a;
    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);
  cdrom_ClearIrq();
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
    }
    return check;
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
    return check;
    tail:
    if (a0 == 0)
    {
      goto loop;
    }
    return 0;
  }
}
