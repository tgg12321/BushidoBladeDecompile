/* REJECTED -- func_80056CB8, s16 (2026-09-16). KILLED (instance), not a cheat --
 * ordinary C restructuring measured worse, not disqualified on cheat grounds.
 *
 * MOTIVATION: the ledger's s6/s7/s15 frontier framed the flags==4 y-compare
 * tail's "beqz+bltz+j triple vs our single bgez" as an open branch-topology
 * question. This session read asm/funcs/func_80056CB8.s:150-192 directly and
 * decoded the branch/delay-slot sequence: it is ordinary MIPS delay-slot fill
 * over the ALREADY-BANKED nested if/else, not evidence of a different C shape.
 * Tried the one plausible untried alternative -- collapsing to a single
 * absolute-value comparison -- to confirm this negatively.
 *
 * FORM TRIED (replacing the banked nested if/else in the flags==4 branch):
 *   s32 y = *(s32 *)(obj + 0xBC);
 *   s32 dy = y - hit1[1];
 *   s32 ady = (dy >= 0) ? dy : -dy;
 *   if (ady >= 0x3E9) {
 *       flags = 5;
 *   }
 *
 * RESULT: score 42 -> 45/204 on the s15-banked 42/197 chassis (build_insns
 * unchanged at 197 -- same real instruction count, worse register/ordering
 * diff). Reverted; re-confirmed 42/197 reproduces exactly after revert.
 *
 * CONCLUSION: the already-banked nested if/else
 *   if (y - hit1[1] >= 0) {
 *       if (y - hit1[1] >= 0x3E9) { flags = 5; }
 *   } else {
 *       if (hit1[1] - y >= 0x3E9) { flags = 5; }
 *   }
 * is the correct C-level shape for this block. Do not re-propose the
 * absolute-value collapse, or any other restructuring of this specific
 * if/else, as a lever for this residual -- the sub-residual here is closed
 * at the C-structure level; any remaining gap traces to surrounding
 * register/delay-slot-fill decisions, not this block's own spelling.
 * ------------------------------------------------------------------- */
