/* REJECTED (s2, 2026-07-27): score 13 (no change from floor). Hypothesis was
 * that `arg0 = arg0;` (dead-store-fake-exception family, FAKE-annotated)
 * between the shift groups would SET the param's reg and make cse.c purge the
 * (lshiftrt arg0 1) hash entry, forcing fresh srls. MEASURED DEAD: build stays
 * 39/41 insns — the same-pseudo self-copy is elided before CSE ever sees a
 * SET (expand emits nothing for pseudo->same-pseudo moves), so no
 * invalidation happens. Do not re-propose self-assign spellings of the param
 * (arg0 = arg0, arg0 += 0, arg0 |= 0 all tree-fold to the same no-op) as a
 * CSE-defeat lever for this function.
 */
void func_80068ECC(s32 arg0) {
    s32 *p = &D_8009BC04;
    s32 v = *p;
    v &= ~0x1; v |= arg0 & 0x1;
    v &= ~0x2; v |= arg0 & 0x2;
    v &= ~0x4; v |= arg0 & 0x4;
    v &= ~0x8; v |= ((u32)arg0 >> 1) & 0x8;
    arg0 = arg0; /* FAKE: intended cse.c invalidation — measured inert */
    v &= ~0x10; v |= ((u32)arg0 >> 1) & 0x10;
    arg0 = arg0; /* FAKE: same */
    v &= ~0x20; v |= ((u32)arg0 >> 1) & 0x20;
    v &= ~0x40; v |= (arg0 << 3) & 0x40;
    v &= ~0x80; v |= arg0 & 0x80;
    *p = v;
}
