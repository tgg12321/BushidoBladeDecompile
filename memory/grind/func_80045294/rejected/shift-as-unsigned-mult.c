/* s5 rejected: (s32)((u32)a0 * 16u) folds identically to a0<<4 at expand-time.
 * score=2, build_insns=83 — NEUTRAL. Same LUID, same shape.
 * KILL: GCC 2.7.2 expand_mult normalizes constant power-of-2 multiplication
 * to (ashift) BEFORE tree_LUID is assigned. Any C spelling that evaluates
 * to a0*16 collapses to the same subtree. Confirms frontier #2 dead angle 1/3.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 v1 = (s32)((u32)a0 * 16u);
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 i = a0;
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;
    /* ... body identical to candidate.c ... */
}
