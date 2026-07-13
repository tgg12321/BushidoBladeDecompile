/* REJECTED — sandbox --disable all == 7. IDENTICAL score to the bare literal.
 *
 * Probe: keep a NAMED constant but let GCC see it as a compile-time constant.
 *
 *     const s32 vel_y = -0xC8;
 *     ...
 *     *(s32 *)(s0 + 0x20) = vel_y;
 *
 * GCC 2.7.2 const-propagates the initializer and the store expands with the
 * literal operand — byte-for-byte the same output as rejected/
 * literal-at-store-site-scores-7.c (li lands inside the mult window, not the
 * entry BB).
 *
 * This closes the whole "name the constant without a mutable pseudo" axis:
 * `const s32` local, `#define`, `enum` constant, file-scope `static const s32`
 * are all the same thing to cc1 — a compile-time constant, folded to the use
 * site. NONE of them can place the materialization in the entry BB.
 *
 * Only a MUTABLE local pseudo, initialized at function entry, survives into
 * the entry basic block. That is the sole C form that reaches the target.
 */
u8 *func_80032064(u8 *src, s32 type) {
    s32 speed = 0x50;
    const s32 vel_y = -0xC8;   /* <-- THE PROBE: const-qualified */
    s32 i = 0;
    u8 *ptr = &D_80104E88;
    u8 *s0;
    s16 sp_area[2];

    for (; i < 4; i++) {
        s0 = ptr;
        if (*s0 == 0) break;
        ptr = s0 + 0x2C;
    }
    if (i == 4) return 0;

    /* ... body unchanged; see candidate.c ... */
    *(s32 *)(s0 + 0x20) = vel_y;
    return s0;
}
