/* Novel 2 (s8): inline D_800A33AC in both loop guards, drop `count` local.
   Hypothesis (from m2c rederive): target's asm loads count into $a0 (scratch)
   in the first loop and RELOADS D_800A33AC after the call for the second loop.
   Suggests source uses D_800A33AC INLINE rather than caching in a local.
   Removing the `count` decl also removes one pseudo from RA priority order.
   Result: score=2 build_insns=83 target_insns=83 — NEUTRAL. GCC's licm hoists
   the load into a pseudo regardless; RTL is identical to cached form. Removing
   the C decl doesn't change codegen. Killed axis: caching D_800A33AC in a local
   vs inlining it is NEUTRAL for this function. Same 2-insn residual.
   Verdict: KILLED (neutral).
*/
