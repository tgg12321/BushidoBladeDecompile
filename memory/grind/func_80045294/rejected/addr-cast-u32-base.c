/* s39 KILLED (NEUTRAL).
 * Address-arithmetic type surface: change `(u8 *)&D_800EED14 + v1` ->
 * `(u32)&D_800EED14 + v1` in s4's initializer, so the base is a
 * pointer-sized INTEGER instead of a `u8 *` pointer.
 * Result: sandbox --disable all -> score=2, target_insns=83, build_insns=83
 * (byte-identical to baseline).
 * Mechanism: GCC 2.7.2 fold reduces both spellings to the same
 * (plus (symbol_ref D_800EED14) (reg:SI a0<<4)) tree at expand time;
 * TYPE_UNSIGNED distinction between (u8*) and (u32) is discarded before
 * tree_LUID is assigned. Consistent with s5's expand-fold-agnostic finding
 * extended to the ADDR-BASE surface. Neutral, not a lever.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 v1 = a0 << 4;
    s32 s4 = *(s32 *)((u32)&D_800EED14 + v1);
    /* ... rest identical to baseline ... */
}
