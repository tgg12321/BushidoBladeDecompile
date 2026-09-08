/* CD_sync candidate - s126 (2026-09-07).  MATCHED: sandbox --disable all = 0/160,
 * build_insns 160, rules_dropped 0, AND full-build verify-oracle ok:true with
 * build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa.  This body is in place
 * in src/system.c.
 *
 * HOW IT CLOSED (the forced-rederive sibling transplant, first probe of the
 * session).  The s117-s125 chassis (CD_alarm struct + honest goto poll loop +
 * combine-foldable chain-extender + folded/split ix arithmetic) was DISCARDED
 * whole, not tweaked.  In its place: CD_ready's matched on-main do_timeout
 * window and CD_datasync's pointer-alias declaration block, transplanted onto
 * CD_sync.  The three functions are the same PsyQ libcd bios.c v1.86 family and
 * their do_timeout windows are instruction-for-instruction the same shape
 * (asm/funcs/CD_sync.s:49-74 vs asm/funcs/CD_datasync.s:48-69).
 *
 * The measurement ladder this session, all on this chassis:
 *   V1  transplant, honest `goto poll` loop, no callback wrap      = 18/160
 *       -- ORDER-EXACT for all 160 instructions.  The 20-session 54/55
 *          transposition that defined the old chassis's residual is GONE; all
 *          18 diffs are callee-saved seat rotations
 *          (ours s1=idx_1494 s2=saved s4=mode s5=result s6=idx_1495,
 *           target s1=saved s2=idx_1494 s4=idx_1495 s5=mode s6=result).
 *   V2  V1 + do-while(0) around the CheckCallback block             =  9/160
 *       -- fixes saved/idx_1494 ($s1/$s2) and tbl_125c ($s3).
 *   V5  V2 + the poll loop written as a REAL `do { ... } while (1);`
 *       with an `if (status == 0) break;` early exit               =  0/160
 *       -- the front end emits NOTE_INSN_LOOP_BEG/END for a real loop, so
 *          flow.c:2081 weights every reference inside it by loop_depth; that
 *          lifts idx_1495's allocno above the two incoming parameters in
 *          global.c's priority sort, seating it in $s4 and pushing mode/result
 *          to $s5/$s6.  The old `goto poll` spelling gets no loop note at all,
 *          which is why 125 sessions of window-arithmetic never reached it.
 *   V7  = V5 minus every construct that ablated byte-neutral: the `new_var`
 *          0xFF constant holder (A2 = 0) and the `tb` ready-byte named
 *          intermediate (A4 = 0) are both DELETED.  V7 = 0/160.
 *
 * Every construct that survives is individually load-bearing, measured this
 * session by single-construct ablation on this chassis:
 *   tbl_125c alias      C1 = 31      idx_1494 alias   C2 = 29
 *   idx_1495 alias      A7 = 12      window do-while  A8 = 25
 *   pp alias            A5 = 18      src staging      A6 =  8
 *   pB address          B1 =  7      arg5 value       B2 =  9
 *   t0 sync byte        B3 = 14      callback wrap    V1 = 18
 * The real `do{}while(1)` poll loop is ORDINARY C, not a FAKE construct: it is
 * the loop the function actually performs, and it is the spelling CD_ready
 * ships matched on main (src/system.c).
 *
 * The BANNED CD_alarm aggregate merge (decisions.md 2026-09-06 11:38) is ABSENT:
 * this body uses HEAD's three flat externs D_800F19B8 / D_800F19BC / D_800F19C0
 * with the `pp` alias, exactly as CD_ready and CD_datasync do.  No declaration
 * surface changes at all - the extern block already on main at src/system.c
 * (D_800A125C[], g_cd_status_a, D_800A11DC[], D_800A11D5, D_800F19B8/BC/C0)
 * is used verbatim, so there is no integration handoff and no scope_allow line.
 *
 * Self-vet: memory/grind/CD_sync/self_vet.md.
 */
s32 CD_sync(s32 a0, u8 *a1)
{
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
  tbl_125c = D_800A125C; /* FAKE: pointer alias (second handle) to the CD_intstr table per pointer-alias-fake-exception (owner ruling 2026-07-01, the `Type* t = &g_Thing;` redundant-second-handle shape), mechanism: global.c seats the base in $s3 across the whole function as the target does (asm/funcs/CD_sync.s:16-17); lever-exhaustion: s126 ablation C1 (direct D_800A125C[] subscript) = 31/160, plus the 125-session ledger in memory/grind/CD_sync/hypotheses.md */
  idx_1494 = &g_cd_status_a; /* FAKE: pointer alias (second handle) to the libcd Intr status block per pointer-alias-fake-exception (owner ruling 2026-07-01); the volatile is the TU's own declaration `extern volatile u8 g_cd_status_a;` (src/system.c, on main since 7e182728; ground truth `static volatile CD_intr Intr`, memory/closer/libcd-identity.md:28) - no cast, no local qualifier; mechanism: global.c seats the base in $s2 across the poll loop as the target does (asm/funcs/CD_sync.s:18-19); lever-exhaustion: s126 ablation C2 (direct (&g_cd_status_a)[n] subscript) = 29/160 */
  idx_1495 = 1 + idx_1494; /* FAKE: second handle (+1) into the same 3-byte Intr block per pointer-alias-fake-exception (NOT cross-symbol arithmetic: D_800A1494/95/96 are one `static volatile CD_intr Intr`, memory/closer/libcd-identity.md:28; the identical idiom ships matched on main at src/system.c CD_ready and cdrom_IrqHandler), mechanism: global.c seats the ready-byte base in $s4 (`addiu s4,s2,1`, asm/funcs/CD_sync.s:20); lever-exhaustion: s126 ablation A7 (idx_1494[1] read at the use site) = 12/160, s105 (three honest respellings of the old chain-extender) = 15 */
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
    void **pp;
    t0 = idx_1494[0]; /* FAKE: named intermediate for the sync byte, placed BEFORE the wrap (loop depth 1), mechanism: flow.c:2081 loop_depth-weighted reg_n_refs feeds local-alloc.c:1660 qty_compare_1 - the depth-1 mention leaves the merged chain-A quantity below the second table read's priority, so the chain takes $a0 and the value $v1 exactly as at asm/funcs/CD_sync.s:49/56/60/65; lever-exhaustion: s126 probe B3 (read inlined into the src address) = 14/160, the 125-session t0 ledger in hypotheses.md s118-s125 */
    do { /* FAKE: do-while(0) wrap per do-while-zero-exception (owner ruling 2026-07-06), mechanism: sched.c:2081 loop-note barrier on the first insn inside (the ready-byte address chain) orders the sync-byte load ahead of it and every later register-argument load after it, and flow.c loop_depth ref weighting seats tbl_125c in $s3; lever-exhaustion: s126 ablation A8 (wrap removed) = 25/160 */
      pB = (s32 *)((idx_1494[1] << 2) + (s32)tbl_125c); /* FAKE: named address intermediate (fresh, once-written, once-read, real value = `addu $v0,$v0,$s3` at asm/funcs/CD_sync.s:56), mechanism: rank_for_schedule INSN_LUID tie-break (sched.c:2462) between the boosted chain-B address insn and the boosted chain-A shift - the address must precede the shift and the value load follow it in RTL order; lever-exhaustion: s126 probe B1 (folded back into the arg5 load) = 7/160 */
      src = (u8 *)((t0 << 2) + (s32)tbl_125c); /* FAKE: chain-A address staged through the (dead-here) src copy-loop variable per staged-value-reused-variable (owner-sanctioned 2026-07-03), mechanism: the multi-set destination keeps the addu unboosted (birthing_insn_p sched.c:2505) so it fills the backward-pass slot behind the sw instead of the shift, and global.c seats it in $a0 with src's copy-loop lives; lever-exhaustion: s126 ablation A6 (fresh local `ta` instead of the reused src) = 8/160 */
      arg5 = *pB; /* FAKE: named intermediate for the fifth (stack) argument (fresh, once-written, once-read, real value = `lw $v1,0($v0)` at asm/funcs/CD_sync.s:60), mechanism: calls.c store_one_arg - a named value is loaded before the call sequence and stored by the sw at the target slot 64; lever-exhaustion: s126 probe B2 (passed as *pB directly) = 9/160 */
      pp = &D_800F19C0; /* FAKE: pointer alias (second handle) to the alarm callback slot per pointer-alias-fake-exception (owner ruling 2026-07-01, the `Type* t = &g_Thing;` redundant-second-handle shape), mechanism: calls.c:1652-1664 expand_call precomputes a register argument whose rtx_cost > 2 into a pseudo inside a loop (preserve_subexpressions_p), whereas `*pp` is a cheap mem(reg) that stays in the call sequence and cse folds the alias back to the target's `lui $a1 / lw $a1` at asm/funcs/CD_sync.s:51-52; lever-exhaustion: s126 ablation A5 (direct D_800F19C0 read) = 18/160; the CD_alarm struct spelling that this candidate carried through s117-s125 is BANNED (decisions.md 2026-09-06 11:38) and is removed here */
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
  temp = (*idx_1494) & 0xFF;
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
