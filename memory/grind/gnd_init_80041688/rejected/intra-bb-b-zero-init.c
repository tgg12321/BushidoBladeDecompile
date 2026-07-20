/* REJECTED — s10 intra-BB pre-color b=0 init: INERT (DCE'd; not a lever)
 *
 * Form (FALSE arm):
 *   b = 0;
 *   r = *((u8*)player+0x18);
 *   g = *((u8*)player+0x19);
 *   b = *((u8*)player+0x1A);
 *   gnd_load_tex(b | ((r<<16)|(g<<8)));
 *
 * Measured s10 sandbox --disable all: score=2, target_insns=82, build_insns=82
 * (byte-identical to baseline). GCC DCE eliminates the `b = 0;` store BEFORE
 * flow.c/life_analysis coalesces defs, so pseudo 78's live range is NOT widened,
 * and sched1's insn_priority hazard-tag on the b-lbu stays boosted (same as
 * baseline). The intra-BB init axis is INERT — cannot fuse pseudo 78, cannot
 * flip lbu emission order.
 *
 * Corollary: the only intra-BB spellings that WOULD fuse pseudo 78 are ones
 * where the initial def survives DCE (i.e. it is live at some point before
 * being overwritten). Any such shape is either (a) a live use that we don't
 * need for the function's semantics (a dead-store-by-any-spelling per
 * [[no-new-park-categories]]) or (b) semantically-meaningful code that has
 * to originate OUTSIDE BB18, which s5 already KILLED (spill-across-call).
 *
 * Kills the "intra-BB fusion" frontier sliver noted in s10 synthesis. The
 * post-branch-pre-color surface is now measured dead, joining loop1-scope
 * fusion (s7 dichotomy) and pre-branch hoist (s5) as case-exhausted.
 */
