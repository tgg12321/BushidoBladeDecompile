/* REJECTED (measured, s5 2026-08-25): case 3 fully inlined with the
 * D_8009947A store FIRST. sandbox --disable all = 24 (build 245/248): with
 * the store already emitted before the loads there are no anti-dependence
 * edges to pin it, sched1 rotates the block differently, the case-34
 * cross-jump tail merge fires again and additional seat diffs appear.
 * Store-LAST (the shipped candidate order, same order case 13 uses) is the
 * form that measures 0. Do not re-propose store-first.
 */
    case 3:
        s1 = s2;
        D_8009947A = 1;
        s6 = (s32 *)((u8 *)s0 + ALIGN4(*(s32 *)((s3 << 2) + (s32)s0 - 8)));
        s4 = (s32 *)((u8 *)s0 + ALIGN4(*(s32 *)((s3 << 2) + (s32)s0 - 4)));
        break;
