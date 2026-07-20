/* s2 variant H — REJECTED, score 17.
 * Second carrier u = { (s32)sp10 (the addiu v0,sp,0x10 temp), arm-2 0x10017 }
 * on top of the candidate form (goal: unboost the addiu stealing a0lw's
 * scheduling window).
 * Failed two ways at once:
 * 1. RA priority inversion: u (4 refs, len 10, pri 8000) allocated BEFORE val
 *    (5714); post-sched1 u's addiu had been hoisted up into the lw/sh region,
 *    overlapping the sp10[0] v0-local -> v0 blocked for u -> u took $a0
 *    (val's target reg) -> val->$a1, D_800A3468-load->$a2: full rotation.
 * 2. The unboosted addiu moved to a non-target slot anyway (sched1 window
 *    dynamics), reordering the whole post-call block.
 * ALLOCDBG: 92=10909->v1, u=8000->a0, val=5714->a1, arg1=4761->v1, 90->a2.
 * Kill lesson: adding a second globalized carrier reshuffles both the
 * conflict graph AND sched1 positions; the coupled fixpoint moved further
 * from target, not closer.
 */
