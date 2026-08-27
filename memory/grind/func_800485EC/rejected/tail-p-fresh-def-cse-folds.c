/* REJECTED (s1, measured 7): the natural single-def tail spelling.
 *
 *     p = tim + 2;
 *     spr->x = x; spr->y = y;
 *     spr->h = ((u16 *)p)[1];
 *     spr->w = *p++;
 *     spr->pixdata = p;
 *
 * cse1's find_best_addr/fold_rtx (lookup_as_function(op0, PLUS)) rewrites
 * (plus p 2)/(p) onto the still-valid tim base: emits lhu 10(s1)/lw 8(s1)
 * and merges the init+increment into a single addiu v1,s1,12 — 67 insns vs
 * target 68 (target keeps addiu v1,s1,8; lhu 2(v1); lw 0(v1); addiu v1,4).
 * Dump-proven: tmp/grind/func_800485EC/s1/ text1b.jump (pre-cse RTL is
 * p-relative) vs text1b.cse (folded). ANY fresh reg+const def of p in the
 * join block with tim un-invalidated folds this way — the whole spelling
 * class is dead. Fix that landed: `tim += 2; p = tim;` (see candidate.c) —
 * the live self-increment kills tim's old cse quantity so there is nothing
 * to fold onto, and the copy keeps the walker in a fresh caller-saved reg.
 */
