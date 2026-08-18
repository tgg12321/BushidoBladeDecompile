/* REJECTED — func_80017848, s2.  score 34, 123 insns (target 127), frame 0x40.
 * "Regime B": no pointer local live across the entry guard, so the preheader DOES
 * re-load ctx+0xC exactly like target's `lw a0,12(s2)`.  But cse then unifies the loop
 * base with the guard's address pseudo, so target's uncoalesced `move a3,a0` copy and
 * its recomputed `addu a0,a1,a3` never appear (2 insns short per loop), and the whole
 * loop register assignment shifts (v0/v1/a0/a1 instead of a0/a1/a2/a3).
 * Every regime-B spelling measured 34 EXACTLY: fresh-ctx guard x {copy local, two-step
 * copy chain, record-pointer guard local, pointer-form base, int-form base, base formed
 * inside vs. outside the guard body, and fully-inline (LICM-hoisted) addressing}.
 * The inline form is the specific kill of "target's copy is a loop.c LICM artifact":
 * LICM does hoist the invariant, and cse still folds it onto the guard address.
 */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *slots;
    u8 *p;
    u8 *base;
    u8 *rec_a;
    u8 *rec_b;
    s32 i;
    s32 dist;

    if (slot_a == slot_b) {
        return 0;
    }
    if (*(s32 *)((slot_a << 6) + (s32) * (u8 **)(ctx + 0xC) + 0x18) >= 0) {
        if (*(s32 *)((slot_b << 6) + (s32) * (u8 **)(ctx + 0xC) + 0x18) >= 0) {
            return 0;
        }
    }

    i = 0;
    if (i < *(s32 *)((slot_a << 6) + (s32)*(u8 **)(ctx + 0xC) + 0x1C)) {
        do {
            if (*(u16 *)((*(u8 *)((slot_a << 6) + (s32)*(u8 **)(ctx + 0xC) + i + 0x24) << 4) +
                         (s32) * (u8 **)(ctx + 0x10) + 0x4) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)((slot_a << 6) + (s32)*(u8 **)(ctx + 0xC) + 0x1C));
    }
    i = 0;
    if (i < *(s32 *)((slot_a << 6) + (s32)*(u8 **)(ctx + 0xC) + 0x20)) {
        do {
            if (*(s16 *)((*(u8 *)((slot_a << 6) + (s32)*(u8 **)(ctx + 0xC) + i + 0x2C) << 4) +
                         (s32) * (u8 **)(ctx + 0x10) + 0x6) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)((slot_a << 6) + (s32)*(u8 **)(ctx + 0xC) + 0x20));
    }

    dist = math_Distance3D((s32 *)(*(u8 **)(ctx + 0xC) + (slot_a << 6)),
                           (s32 *)(*(u8 **)(ctx + 0xC) + (slot_b << 6)));
    link = *(u8 **)(ctx + 0x10) + (*(s16 *)(ctx + 0x6) << 4);
    *(s32 *)(link + 0x0) = dist;
    *(s32 *)(link + 0x8) = dist * 3;
    *(s32 *)(link + 0xC) = arg1;
    *(s32 *)(link + 0x4) = (slot_a << 16) | slot_b;

    rec_a = (u8 *)((slot_a << 6) + (s32) * (u8 **)(ctx + 0xC));
    i = *(s32 *)(rec_a + 0x1C);
    *(s32 *)(rec_a + 0x1C) = i + 1;
    *(u8 *)(rec_a + i + 0x24) = *(u16 *)(ctx + 0x6);

    rec_b = (u8 *)((slot_b << 6) + (s32) * (u8 **)(ctx + 0xC));
    i = *(s32 *)(rec_b + 0x20);
    *(s32 *)(rec_b + 0x20) = i + 1;
    *(u8 *)(rec_b + i + 0x2C) = *(u16 *)(ctx + 0x6);

    *(s16 *)(ctx + 0x6) = *(u16 *)(ctx + 0x6) + 1;
    return 1;
}
