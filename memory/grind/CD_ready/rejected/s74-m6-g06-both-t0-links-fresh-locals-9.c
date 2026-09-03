/* s68 UPDATE (2026-09-01, escalation). This body is UNCHANGED and remains the floor at masked 2
 * (re-verified live this session: score 2, build 179, target 179, rules_dropped 0).
 * s68 killed BOTH of the s67 frontier axes with dump-level mechanism and discharged the owner's
 * 2026-09-01 Ruling A row for this function in full, so the item was FORECLOSED under the standing
 * ruling (docs/grind/decisions.md, 2026-09-01 CD_ready entry). What a future session must know:
 *   1. KILLED: `a1v` cannot be moved out of local-alloc by any C respelling. Declaring it at
 *      FUNCTION scope produces a local-alloc dump line-for-line identical to d01's (same
 *      quantities, refs, `used` sets, seats; only pseudo numbers shift) - block_alloc decides
 *      membership from REG_BASIC_BLOCK, not C scope. Hard reg 4 is in reg98's `used` because $a0
 *      is the printf call's first argument, NOT because of a1v (which takes $a1). The s67 frontier
 *      item's premise was false.
 *   2. KILLED: folding the arg5 address chain is byte-identical to d01 (7/179) and does not move
 *      the 51/52 lbu pair. Source-level folding cannot shorten an RTL dependence chain; removing a
 *      link costs an instruction (fully-inline = 180 insns), off the 179-parity basin.
 *   3. KILLED: the Ruling D CD_intr aggregate. Prong (c) is structurally unsatisfiable (five
 *      asm-only consumers of D_800A1494/95/96: getintr, CD_cw, func_800817A0, func_800819C4,
 *      func_80081E1C, plus the asm data definition), and the aggregate measures 41/173 (volatile)
 *      and 37/172 (plain) against the floor of 2/179.
 * The ONE open question on this body is still F3 (the volatile prong-2 / allowlist ruling for
 * idx_1496) - documented below and moot while the floor is 2.
 */
/* s67 UPDATE (2026-09-01, solver). This body is UNCHANGED and remains the floor at masked 2
 * (re-verified live this session: score 2, build 179, target 179, rules_dropped 0). It holds the
 * SEATS with a single adjacent ALU transposition left.
 *
 * s67 did NOT improve the floor, but it moved the frontier off this body onto a different base.
 * Read memory/grind/CD_ready/hypotheses.md s67 before working here. The three results that matter:
 *   1. KILLED: an EMPTY do{}while(0) is byte-neutral NOWHERE in the do_timeout block (six
 *      positions, 9/7/7/10/11/10 against the base's 6). The loop note is a sched1 region boundary
 *      here, so a reference count can never be bought for free on this function.
 *   2. CONFIRMED, new axis: SPLITTING the existing tbl_125c wrap so `t0 *= 4` falls into a bare
 *      loop-depth-1 gap LOWERS the t0 shift temp's reg_n_refs and flips the seats
 *      (memory/grind/CD_ready/progress/s67-y02-seats-correct-base.c, score 6).
 *   3. CONFIRMED by pre-registered prediction: on the order-perfect d01 base, raising the arg5
 *      VALUE quantity's refs to exactly 6 (two extra nesting levels on the arg5 load) gives it
 *      $v1, the target's seat (progress/s67-e02-arg5-value-v1.c, score 8).
 * The new frontier base is progress/s67-d01-order-perfect-base.c (score 7, 179 insns) whose
 * instruction SEQUENCE is the target's from insn 53 to the end; its residual is purely seats.
 */
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
 * S65 CORRECTION TO THE PARAGRAPH ABOVE (read this before spending a session on the seats).
 * The s61-s63 model of the seat half (reg 98's ".lreg used 8 times across N insns" window) is a
 * PROXY and it is wrong: reg 98 is not one of local-alloc's block-3 quantities at all. The
 * instrumented cc1 (`bash tmp/grind/CD_ready/s63/qty.sh <out>`, BB2_QTY_DEBUG/BB2_SUGG_DEBUG)
 * prints the real thing - every quantity's qty_compare_1 inputs, its rank, and the hard register
 * find_free_reg gave it. Measured on THIS body: 104 arg5-ADDRESS 18-20 refs4 pri 4.00 -> $v0;
 * 110 22-30 refs8 3.00 -> $v0; 97 arg5-VALUE 20-26 refs4 1.33 -> $v1; 102 t0 16-24 refs4 1.00 ->
 * $a0 (all four seats as the target wants them). On the order-perfect base the arg5 address
 * quantity stretches to span 2 (pri 2.00) because sched1 fills the addu->load latency slot with
 * the t0 shift, and t0 then beats the arg5 value on a 1.33-vs-1.33 tie broken by quantity number.
 * s66 CORRECTION - do not spend the s65 frontier probe. s65 named the closing change as
 * "exchange sched1 insns 145 and 137"; reading k03.sched.txt against the QTYDBG columns shows
 * insn 145 is the death of NO local quantity (reg 98 = the `t0` variable is set twice in the
 * block, so REG_N_DEATHS == 2 and local_alloc skips it; global-alloc seats it, correctly, at
 * $a0). The tie is between reg 104 (the t0 SHIFT temp, insn 117 -> insn 122) and reg 97 (the
 * arg5 VALUE, insn 113 -> insn 137). With the target's instruction sequence held, both pseudos'
 * births, deaths and quantity numbers are forced and both carry the minimum two mentions, so
 * qty_n_refs - the loop-depth-weighted reg_n_refs - is the residual's ONLY free variable. s66
 * demonstrated that a do-while(0) loop note really does move it (arg5 value 4 -> 5 or 6), but
 * every placement tried also perturbs sched1 (best 8). See evidence.md/hypotheses.md s66.
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
  do { /* FAKE: do-while(0) loop-note ref weighting seats tbl_125c in s5 (SOTN FAKE-class match device; do-while-zero-exception 2026-07-06) */
  tslTm2LoadImage_2(&D_800161B8);

  {
    s32 arg5;
    s32 t0;
    void **pp;
    s32 tsh;
    s32 taddr;
    pp = (void **)&D_800F19C0; /* FAKE: pointer-alias staging (staged-value-reused-variable / pointer-alias, owner-sanctioned 2026-07-03) - the D_800F19C0 load placed early so the a1 arg loads at the target slot */
    t0 = idx_1494[0];
    v0 = idx_1494[1]; /* FAKE: index staged through the (dead-here) v0 var per staged-value-reused-variable (owner-sanctioned 2026-07-03); v0's prior value is dead (re-set below before any read) */
    v0 <<= 2; /* FAKE: continued staging per staged-value-reused-variable */
    arg5 = *(s32 *)(v0 + (s32)tbl_125c);
    tsh = t0 * 4;
    taddr = (s32)((u8 *)tbl_125c + tsh);
    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)taddr, arg5);
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
