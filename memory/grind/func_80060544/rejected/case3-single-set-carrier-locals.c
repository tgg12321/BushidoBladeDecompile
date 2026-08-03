/*
 * REJECTED (s6, forensics) — "give the Case3 `la` a SINGLE-SET destination
 * pseudo by using a per-arm local instead of the shared `stat` carrier".
 *
 * WHY IT WAS TRIED.  s6 measured (BB2_SCHED_DEBUG ready-list dumps, see
 * tmp/grind/func_80060544/s6/) that in the winning variant the Case3 `la` insn
 * enters the sched1 ready list carrying LAUNCH_PRIORITY (0x7f000001) instead of
 * the universal priority 1, i.e. adjust_priority() -> birthing_insn_p()
 * promoted it (tools/gcc-2.7.2/sched.c:2566 and :2496).  birthing_insn_p's
 * predicate is `bb_live_regs[dest] && reg_n_sets[dest] == 1`, so the obvious
 * clean C lever is to make the pseudo that the Case3 `la` writes a single-set
 * pseudo — which, in pure C with no dead stores, means giving Case3 its own
 * address local instead of sharing `stat` with the other two arms.
 *
 * MEASURED (tmp/grind/func_80060544/s6/arm_sweep.py, reduced-TU cc1, the
 * cheat-free flag set; base = 117 asm lines):
 *
 *   W0_base            a1-after   dest=r75  nsets=3  launch=0  asm_lines=117
 *   W1_case3_own       (a1 gone)  dest=r76  nsets=1  launch=0  asm_lines=119
 *   W2_three_locals    (a1 gone)  dest=r76  nsets=1  launch=0  asm_lines=122
 *   W3_others_own      (a1 gone)  dest=r75  nsets=1  launch=0  asm_lines=122
 *   W4_case3_own_ptr   (a1 gone)  dest=r76  nsets=1  launch=0  asm_lines=119
 *   W9_reuse_ctl       A1-FIRST   dest=r77  nsets=2  launch=1  asm_lines=117
 *
 * RESULT: every per-arm-local form DOES make the destination a single-set
 * pseudo and still does NOT fire the promotion (launch=0), and every one of
 * them costs +2 to +5 instructions — the shared `stat` carrier that s1's H2
 * introduced (floor 15 -> 8) is load-bearing and cannot be split per arm.
 * W9 (the judge-FAILed staging form) is the positive control: it flips with the
 * SAME instruction count, which is how the harness is validated.
 *
 * s6 then established the real predicate (see hypotheses.md H-F5): the flip
 * needs the carrier to be DEAD after the p_static store (so flow/combine
 * retargets the `la`'s destination onto the carrier's pseudo and deletes the
 * copy) AND to have exactly ONE other assignment in the function (so the
 * decremented reg_n_sets lands on 1).  Single-set-ness alone is neither
 * necessary nor sufficient.
 *
 * DO NOT RE-PROPOSE any of these.  They are strictly worse than the candidate
 * and the mechanism now explains why.
 */

/* W1 — Case3 gets its own local (the other two arms keep the shared carrier). */
    s32 stat;
    s32 stat3;          /* + this declaration */
    ...
    S7D8:
        stat = (s32)(&D_8009B7D8);
        s.p_static = (s32 *)stat;
        goto Skip;
    S800:
        stat = (s32)(&D_8009B800);
        s.p_static = (s32 *)stat;
        goto Skip;
    Case3:
        stat3 = (s32)(&D_8009B7D0);     /* single-set pseudo */
        s.p_static = (s32 *)stat3;
        s.pad0C = mid_off;
        mid_off = func_80073728(&s, 0);
/* -> 119 asm lines (base 117), launch=0.  KILLED. */

/* W2 — every arm gets its own local: stat0 / stat1 / stat3.  122 lines. KILLED. */
/* W3 — the OTHER two arms get their own locals so the shared `stat` becomes
 *      single-set and written only in Case3.  122 lines, launch=0.  KILLED. */
/* W4 — W1 with a pointer-typed local (`s32 *stat3; stat3 = &D_8009B7D0;
 *      s.p_static = stat3;`), no casts.  119 lines, launch=0.  KILLED. */
