/* REJECTED (measured 2026-08-25 [s4]): with the banned case-3 volatile
 * removed, every STATEMENT-ORDER respelling of case 3 measured flat at
 * sandbox 9 (245/248) — the scheduler places li/sh(D_8009947A) between the
 * two ptr[-N] loads under every legal order, m1 lands in $v0, and case 3's
 * ALIGN4 tail (srl/sll/addu $20,$16,$2) becomes byte-identical to case 34's,
 * so jump2 find_cross_jump merges the 3-insn suffix (case 3 ends j .L31 into
 * case 34's tail). Probes, all flat at 9:
 *   P1: D_8009947A = 1; moved AFTER the s6/s4 computes (store last)
 *   P2: D_8009947A = 1; between the s6 and s4 computes
 *   P3: read order swapped (raw_m1 first, raw_m2 second)
 *   P4: case-13-style spelling (s32 off1 = ALIGN4(ptr[-2]); s32 off2 =
 *       ALIGN4(ptr[-1]); ... s6 = s0+off1; s4 = s0+off2;)
 * Solver kill (stronger): sched_solver perturb.py on the case-3 block
 * (pass 1, block 19, goal = target order 298,301,303,306,309,312,314,316,
 * 317,319,321,322,324,328) is UNREACHABLE at depth 2 with ALL atom classes
 * (584 atoms) and at depth 3 with the spellable luid/luid_move atoms (273):
 * NO reordering of THIS RTL's statements reaches target order. The original
 * RTL must carry a dependence edge ours lacked — confirmed by the closing
 * form ([s4] candidate): plain-pointer-var derefs (pm2/pm1) emit non-struct
 * MEMs, sched.c then raises REG_DEP_ANTI(load -> D_8009947A store) edges and
 * target order follows from real dependences.
 *
 * Representative flat form (P4):
 *   s32 *ptr = (s32 *)((s3 << 2) + (s32)s0);
 *   s32 off1 = ALIGN4(ptr[-2]);
 *   s32 off2 = ALIGN4(ptr[-1]);
 *   D_8009947A = 1;
 *   s6 = (s32 *)((u8 *)s0 + off1);
 *   s4 = (s32 *)((u8 *)s0 + off2);
 */
