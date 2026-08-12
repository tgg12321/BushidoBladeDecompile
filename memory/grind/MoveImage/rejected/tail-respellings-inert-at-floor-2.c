/* REJECTED (grind session 2, structural) — every respelling of the DISPATCH
 * TAIL and of the argument/type views. All are byte-identical to the floor-2
 * candidate; the p[6] load's schedule slot does not move.
 *
 * 29 measured forms, all score 2 / build_insns 49 (i.e. inert):
 *   tail spellings  — inline dispatch `((s32(*)())p[2])(p[6], ...)`;
 *                     p[6] staged into a local before or after `fn`;
 *                     `(s32)bf24 - 8` staged into a local; `*(p + 6)`;
 *                     `(s32)(bf24 - 2)`; `(*fn)(...)`; result staged into a
 *                     local and returned; dev table re-read for `fn` and read
 *                     again for `p`; declaration order of p/fn/packed/bf24.
 *   dev-table placement — `p =` (and `fn = p[2]`) placed before the stores,
 *                     between the stores, first statement of the body: all 2/49
 *                     EXCEPT `p =` AND `fn =` both before the stores (13/48,
 *                     loses the steal).
 *   type views      — u32* dev-table local in the LoadImage/StoreImage sibling
 *                     spelling; fully prototyped fn pointer; s16* rect param
 *                     with arg0[2]/arg0[3] guard; s32 x/y params; u32* primitive
 *                     pointer; u32* walked rect; `20` / `(s32)0` constants;
 *                     named s16* guard pointer.
 *
 * CONCLUSION: the residual is NOT reachable from the tail or from the type
 * view. It is a cc1 sched.c priority fact about insn 91 (the p[6] load) vs
 * insn 80 (the BF2C store) — see hypotheses.md H7. Anything that only changes
 * how the dispatch is written is wasted effort; do not re-run this axis.
 *
 * Forms that were WORSE than the candidate (so also dead):
 *   packed operands swapped `((u32)arg1 & 0xFFFF) | ((s32)arg2 << 16)`   8/49
 *   u16 arg1 param, unmasked                                             9/49
 *   dev-table split into two pointers (q = p + 6)                        4/49
 *   `p = g_gpu_dev_table + 6; fn = p[-4]; fn(*p, ...)`                   4/49
 *   primitive pointer post-incremented at its store (`*bf24++`)          9/49
 *   BF24 written as a plain symbol store, pointer kept only for the call 12/49
 *   rect[1] read as `rect[1]` with no post-increment on rect[0]         21/47
 */
