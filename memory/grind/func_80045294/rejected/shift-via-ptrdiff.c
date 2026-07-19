/* s5 rejected: (s32)((u8*)0 + a0) * 16 — pointer-diff cast route.
 * score=2, build_insns=83 — NEUTRAL. GCC folds (u8*)0 + a0 to a0 immediately;
 * the outer *16 then folds to <<4 identically.
 * KILL: no LUID shift; frontier #2 dead angle 3/3.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 v1 = (s32)((u8 *)0 + a0) * 16;
    /* ... */
}
