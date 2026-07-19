/* s5 rejected: s32 v1 = a0 * 16 — folds identically to a0<<4.
 * score=2, build_insns=83 — NEUTRAL.
 * KILL: same expand_mult fold as shift-as-unsigned-mult.c.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 v1 = a0 * 16;
    /* ... */
}
