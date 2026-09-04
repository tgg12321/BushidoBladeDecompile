/* REJECTED (s44, synthesis / SIBLING SWEEP). Axis A, second F1 spelling.
 *   s32 *bf68 = D_8009BF68;
 *   printf(&D_80016044, bf68[(s32)((u8 *)bf68 - (u8 *)&D_8009BF68)], ...);
 * A self-relative zero delta off a local alias of the array, so the address
 * appears twice in the expression tree. Measured: score 13, build_insns 90 --
 * same outcome as the BF6C-base extender: the fold at the BF68 slot survives.
 * (A third spelling, base &D_80016044 + delta, folds away entirely: score 9,
 * build 90 -- inert, banked in hypotheses.md rather than here.) */
