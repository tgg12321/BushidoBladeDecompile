/* s18 rederive KILLED — score 3 (byte-neutral) BUT this axis was not
 * previously measured in this exact spelling and is worth banking so
 * future sessions don't re-derive it.
 *
 * Fresh m2c decompile produced the combined shift idiom:
 *     (s32)(x << 0x10) >> 0xE
 * which is algebraically identical to candidate.c's
 *     ((s32)(x << 16) >> 16) << 2
 * but presents combine.c a different AST shape (single arithmetic shift
 * right vs shift-right-then-shift-left).
 *
 *     p = (s16 *)(((s32)(prev_idx << 16) >> 14) + (s32)table);
 *
 * Result: sandbox --disable all = 3 (byte-neutral). GCC 2.7.2 combine.c
 * folds both spellings to the same RTL. Extends s2's spelling-fold finding
 * (which tested `((s32)(s16)prev_idx)<<2`) to the third variant.
 *
 * Not a lever. This axis is closed at every spelling variant tested so far.
 */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    /* ...prologue same as candidate.c... */
    /* p1 slot: */
    /*   p = (s16 *)(((s32)(prev_idx << 16) >> 14) + (s32)table); */
    /* p2 slot: */
    /*   p = (s16 *)(((s32)(next_idx << 16) >> 14) + (s32)(*(s16 **)(arg0+4))); */
}
