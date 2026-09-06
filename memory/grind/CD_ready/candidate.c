/*
 * CD_ready (splat marionation_Exec) - src/system.c - Sony libcd CD_ready(int mode, u_char *result)
 * ==============================================================================================
 * STATE (s88d, 2026-09-06): BYTE MATCH, 0/179/179 with `sandbox CD_ready --disable all`
 * (rules_dropped 0) and `verify-oracle` ok:true, WITHOUT the banned CD_alarm struct.
 *
 * HOW TO APPLY (HEAD carries INCLUDE_ASM("asm/funcs", CD_ready); per asm-until-matched):
 *     python3 memory/grind/CD_ready/apply_s78.py memory/grind/CD_ready/candidate.c
 * The three //directives at the top of this file are the declaration-surface edits
 * (one added `extern volatile u8 g_cd_status_a;` ahead of the CD_ready marker - the TU's own
 * declaration at system.c:400, hoisted; the MMIO index-register pointer declared volatile at
 * its existing extern; the forbidden asm("D_800A147C") alias-rename REMOVED). The body uses
 * the real symbol names, so the RENAME map in apply_s78.py is a no-op and the body hash of
 * this file equals the body hash of src/system.c after apply (s88c keying lesson).
 *
 * LINEAGE: s88 H3 (first 0/179, derived from the s80-s87 floor traces - evidence.md [s88])
 *   -> s88b VB (the status-byte pointers typed by the TU's `extern volatile u8 g_cd_status_a;`
 *      declaration instead of a local volatile qualifier; Judge PASS 2026-09-06 11:33)
 *   -> s88c submission FAILED layer-1 (2026-09-06 11:38) on the CD_alarm struct
 *      (`typedef struct { s32 timeout; s32 count; void *func; } CD_alarm; extern CD_alarm
 *      D_800F19B8;`), now a BANNED construct for this function
 *   -> s88d (THIS FILE): the alarm block is back on HEAD's three flat externs
 *      `extern s32 D_800F19B8; extern s32 D_800F19BC; extern void *D_800F19C0;` (no
 *      declaration change), and the printf's second argument is read through a local
 *      pointer alias `void **pp = &D_800F19C0;` (pointer-alias-fake-exception family), which
 *      is exactly what the struct had been doing for the codegen (candidate.c s78 header:
 *      "it makes the `void **pp` pointer-alias FAKE non-load-bearing"). Measured this session:
 *        F1  flat externs, printf reads D_800F19C0 directly ........ 23/180  (rejected/)
 *        F2a pp assigned after t0, before the outer wrap ............  0/179  (progress/)
 *        F2b pp assigned inside the wrap after tb ...................  0/179
 *        F2c pp assigned as the wrap's first statement ..............  0/179
 *        F2d pp function-scope, assigned at the top ................. 10/183  (rejected/)
 *        F2e pp assigned after pB ...................................  0/179
 *        F2f pp assigned right before the printf (THIS BODY) ........  0/179
 *      Mechanism (dumps tmp/grind/CD_ready/s88/F1.* vs F2f.*): calls.c:1652-1664 - inside a
 *      loop expand_call copies a register argument with rtx_cost > 2 into a pseudo BEFORE the
 *      call sequence; `mem(symbol_ref D_800F19C0)` qualifies (F1.combine insn 119 `(set (reg 106)
 *      (mem (symbol_ref D_800F19C0)))` far ahead of the chains, then insn 135 `(set a1 (reg 106))`),
 *      `*pp` = `mem(reg)` does not, and cse then folds pp's constant address back in, giving
 *      F2f.combine insn 136 `(set (reg a1) (mem (symbol_ref D_800F19C0)))` in the call sequence
 *      = the target's `lui a1 / lw a1,0(a1)` at slots 53-54.
 *
 * FAKE UNITS IN THIS BODY (each ablated and banked - hypotheses.md s88/s88b/s88d):
 *   tbl_125c, idx_1494/1495/1496, pp  pointer aliases (pointer-alias-fake-exception)
 *   t0 placement before the wrap; outer do-while(0) wrap (do-while-zero-exception)
 *   tb, pB, arg5 named intermediates (ordinary-c-judge-decidable, named-intermediate)
 *   src staged chain-A address (staged-value-reused-variable)
 *   new_var/new_var3 constant holders (named-local-fake-exception)
 *   nested + single do-while(0) clear wraps (do-while-zero-exception, prerequisite 3 note)
 * NOT fake: the volatile type of g_cd_status_a (TU declaration on main since 7e182728,
 * consumed by matched CD_flush / cdrom_IrqHandler; Sony ground truth `static volatile
 * CD_intr Intr`, memory/closer/libcd-identity.md:28) and the MMIO-range D_800A147C
 * (mmio-volatile-type-level).
 *
 * The `src = (u8 *)&D_800F19B0;` / `(u8 *)&D_800F19A8` casts take a byte pointer to objects
 * HEAD declares as `extern void D_800F19xx;` (system.c:361-363) - the same handles the
 * matched cdrom_IrqHandler passes as `&D_800F19A8` / `&D_800F19A0` (system.c:631/636); they
 * are the three 8-byte libcd Result buffers (memory/closer/libcd-identity.md:32), not a
 * per-word split of one buffer. No declaration is changed by this candidate.
 *
 * Full derivation and every measurement: memory/grind/CD_ready/evidence.md [s88]..[s88d],
 * memory/grind/CD_ready/hypotheses.md.
 */
//INS_BEFORE:extern u8 D_800A1494;|extern volatile u8 g_cd_status_a;
//REPLALL:extern u8 *D_800A147C; => extern volatile u8 *D_800A147C;
//DROPALL:extern volatile u8 *D_800A147C_2 asm("D_800A147C");
s32 CD_ready(s32 a0, u8 *a1)
{
  s32 v0;
  s32 cnt;
  volatile u8 *idx_1494;
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
  tbl_125c = D_800A125C; /* FAKE: pointer alias to the CD_intstr table per pointer-alias-fake-exception, mechanism: the base is held in s5 across the poll loop as in the target; lever-exhaustion: direct-subscript spellings s66 s07 (14) */
  idx_1494 = &g_cd_status_a; /* FAKE: pointer alias to the libcd Intr status block per pointer-alias-fake-exception (owner ruling 2026-07-01); the pointer's volatile type is the TU's declaration `extern volatile u8 g_cd_status_a;` (src/system.c, on main since 7e182728; ground truth `static volatile CD_intr Intr`, memory/closer/libcd-identity.md:28) - no cast, no local qualifier; mechanism: the base is held in s2 across the poll loop as in the target; lever-exhaustion: type-level direct spellings s60 (4/180), s88 VD (&g_cd_status_c: 2/180) */
  idx_1495 = 1 + idx_1494; /* FAKE: second handle (+1) per pointer-alias-fake-exception, mechanism: base register s6 for the ready byte in the callback block as in the target (the idiom cdrom_IrqHandler ships on main: `volatile u8 *s3 = s1 - 1;`, src/system.c:766); lever-exhaustion: s60 */
  idx_1496 = idx_1494 + 2; /* FAKE: third handle (+2) per pointer-alias-fake-exception, mechanism: base register s3 (`addiu s3,s2,2`) for the completion byte as in the target; the volatile store `*(idx_1496 - 1) = 0` in check2 is load-bearing through reorg.c:760 (resource_conflicts_p: a volatile store sets set.volatil, so fill_simple_delay_slots takes nothing into the `beqz a2` slot - the target's nop); lever-exhaustion: s88 R1 (non-volatile idx_1496: 2/178, reorg hoists `move a1,s4` into the slot) */
  D_800F19BC = 0;
  D_800F19C0 = &D_80016248;
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
    t0 = idx_1494[0]; /* FAKE: sync-byte read placed before the wrap (loop depth 1), mechanism: flow.c loop_depth-weighted reg_n_refs feeds local-alloc.c:1660 qty_compare - the depth-1 mention (qty refs 7, not 8) puts the merged chain-A quantity below the arg5 value's priority, so the value takes $v1 and the chain $a0; lever-exhaustion: s66 s01-s08, s88 N1/V2n/G2 (chain inside the wrap, refs 12: score 15) - hypotheses.md s88 */
  do { /* FAKE: do-while(0) wrap, mechanism: sched.c:2081 loop-note barrier on the first insn inside (the ready-byte load) orders the sync-byte load ahead of it and every later register-argument load after it, and flow.c loop_depth ref weighting seats tbl_125c in s5 (SOTN FAKE-class match device; do-while-zero-exception 2026-07-06); lever-exhaustion: wrap ablated s81 2 -> 12, s88 H4 0 -> 30 */
    tb = idx_1494[1]; /* FAKE: named intermediate for the ready byte (fresh, once-written, once-read, real value = lbu v0,1(s2)), mechanism: expand argument staging - with arg5 it keeps the stack argument's chain out of the call sequence; lever-exhaustion: s88 H6 (tb and arg5 inlined into the printf call) = 9 */
    pB = (s32 *)((tb << 2) + (s32)tbl_125c); /* FAKE: named address intermediate (fresh, once-written, once-read, real value = addu v0,v0,s5), mechanism: rank_for_schedule INSN_LUID tie-break (sched.c:2462) between the boosted chain-B address insn and the boosted chain-A shift - the address must precede the shift and the value load follow it in RTL order; lever-exhaustion: s88 H5 (folded back into the value load) = 2 = the s80-s87 floor */
    src = (u8 *)((t0 << 2) + (s32)tbl_125c); /* FAKE: chain-A address staged through the (dead-here) src var per staged-value-reused-variable (owner-sanctioned 2026-07-03), mechanism: the multi-set destination keeps the addu unboosted (birthing_insn_p sched.c:2505) so it fills the backward-pass slot behind the sw instead of the shift, and global.c seats it in $a0 with src's copy-loop lives; lever-exhaustion: s87 F4/F5 (fresh ta: 9), s88 N1/V2n/G2 (in-place t0: 15), s87 P5a/P5b (5/9) */
    arg5 = *pB; /* FAKE: named intermediate for the fifth (stack) argument (fresh, once-written, once-read, real value = lw v1,0(v0)), mechanism: calls.c store_one_arg - a named value is loaded before the call sequence and stored by the sw at the target slot; lever-exhaustion: s88 H6 (passed as *pB directly) = 9 */
    pp = &D_800F19C0; /* FAKE: pointer alias (second handle) to the alarm callback slot per pointer-alias-fake-exception (owner ruling 2026-07-01, the `Type* t = &g_Thing;` redundant-second-handle shape), mechanism: calls.c:1652-1664 expand_call precomputes a register argument whose rtx_cost > 2 into a pseudo inside a loop (preserve_subexpressions_p) - the bare `mem(symbol_ref D_800F19C0)` is copied to a pseudo before the chain-A/chain-B insns and the sw (F1.combine insn 119 + `move a1`), whereas `*pp` is a cheap `mem(reg)` that stays in the call sequence and cse folds the alias back to `(set a1 (mem (symbol_ref D_800F19C0)))` (F2f.combine insn 136), the target's `lui a1/lw a1` at slots 53-54; lever-exhaustion: direct global read s88d F1 = 23/180 (a1 load displaced to slots 61-62, seats shuffled), s53-s57 scalar model = 9, the CD_alarm struct spelling is BANNED (decisions.md 2026-09-06 11:38); placement inside the block is byte-inert (s88d F2a/F2b/F2c/F2e/F2f all 0/179), function-scope placement F2d = 10/183 (pp becomes a loop-carried callee-saved live range) */
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

  new_var = 0xFF;  /* FAKE: opaque mask variables (with new_var3) keep the target's redundant `andi ,0xff` alive (named-local constant-holder family). Alternatives exhausted and recorded in memory/wip/CD_ready/notes.md: u8-typed checks fold via PROMOTE_MODE+combine (measured 17), staged raw byte folds (proven byte); the symbolic mask is the one spelling combine cannot fold */
  new_var3 = 0xFF;
  do { /* FAKE: do-while(0) loop-note ref weighting seats idx_1494/idx_1495 in s2/s6 */
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
    *D_800A147C = saved;
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
