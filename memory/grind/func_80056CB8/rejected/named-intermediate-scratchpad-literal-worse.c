/* s15 REJECTED — worse (46/204, build_insns 199 vs the 42/197 baseline).
 * Named the repeated 0x1F8002B8 scratchpad-address literal as a single
 * fresh local instead of writing it at both func_80053614() call sites —
 * this is exactly the "single named intermediate" lever
 * tools/ra_solver/inverse_compose.py classify's own C-lever list names for
 * this PRE-RA residual. Measured worse: forcing the literal into one
 * pseudo live across the intervening func_80053614 call added a spill
 * beyond what the baseline's two `sw s8,16(#)` per-call stack-arg stores
 * (o32 ABI 5th-argument convention) already cost. Full writeup:
 * memory/grind/func_80056CB8/candidate.c s15 header, "TRIED AND KILLED
 * THIS SESSION" section.
 *
 * Only the changed region is shown; splice into the s14-banked
 * candidate.c body to reproduce.
 */

/* loop-body locals: added */
s32 scratchpad;

/* loop-body top: added, right after the `s32 z;` declaration */
scratchpad = 0x1F8002B8;

/* first func_80053614 call site: changed literal -> local */
flags = func_80053614(pt0, pt1, (s32)hit0, (s32)work, scratchpad);
/* (was: ..., (s32)work, 0x1F8002B8); */

/* second func_80053614 call site: changed literal -> local */
flags = (flags | (func_80053614(pt0, pt1, (s32)hit1, (s32)work, scratchpad) << 1)) + 1;
/* (was: ..., (s32)work, 0x1F8002B8) << 1)) + 1; */
