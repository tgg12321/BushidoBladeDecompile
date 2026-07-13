/* REJECTED — s4, 2026-07-13.  The volatile path through true_dependence is CLOSED.
 * (Measured to close the search space, NOT proposed: volatile coercion is a
 *  catalogued cheat unless the memory is genuinely MMIO / interrupt-touched, and
 *  there is no evidence this region is.)
 *
 * sched.c's true_dependence has exactly TWO ways to report a conflict:
 *
 *   return ((MEM_VOLATILE_P (x) && MEM_VOLATILE_P (mem))          <-- path 2
 *           || (memrefs_conflict_p (...)                          <-- path 1
 *               && ! (MEM_IN_STRUCT_P ... && rtx_addr_varies_p ...)
 *               && ! (...)));
 *
 * PATH 1 (shared base symbol) is killed in
 *   rejected/aggregate-any-spelling-loses-offset0-atform.c — every honest spelling
 *   that shares the base symbol also forces a shared base REGISTER, which costs the
 *   three at-form `lui`s and changes the block copy to num_regs=3.
 *   (The two MEM_IN_STRUCT_P clauses are inert: both require rtx_addr_varies_p,
 *   which is false for symbol addresses.)
 *
 * PATH 2 (both mems volatile) is killed HERE, measured in
 *   tmp/grind/func_8001C624/s4/probe_volatile.c:
 *
 *   n_vol       — volatile region scalars, ordinary block copy:
 *                 the copy STILL floats above the +4/+8 stores, byte-identical to
 *                 the non-volatile form.  true_dependence needs BOTH mems volatile,
 *                 and the copy's mem is not.  No fence.  Useless.
 *
 *   n_vol_both  — both sides volatile (`*(volatile Blk12 *)&DST =
 *                 *(volatile Blk12 *)&VS0`):
 *                 GCC 2.7.2 REFUSES to inline a volatile block move and emits
 *                     jal memcpy
 *                 instead.  The target has an inline lw/lw/sw/sw block move and no
 *                 call.  Structurally wrong shape — cannot match, at any score.
 *
 * So there is no volatile spelling that both fences the copy and keeps it inline.
 * Both of true_dependence's paths are now exhausted, which is what makes the s4
 * exhaustion argument airtight rather than merely "we tried a lot of things".
 */
