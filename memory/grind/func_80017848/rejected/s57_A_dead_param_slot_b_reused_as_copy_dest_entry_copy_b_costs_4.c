/* REJECTED s57 (2026-09-15): cell A = 4 at 127/127, byte-identical to K2. K2 + `b = slot_b;` as the first statement (every later slot_b use through b) and the loop copy destination spelled as the now-dead parameter (`slot_b = (s32)q; q = (u8 *)(sh + slot_b);`), intended to give the copy dest the incoming-argument a3 copy preference (set_preference, global.c:1671). The .greg dump shows pseudo 75 (slot_b) with `conflicts: 72 73 74 75 79 81 84 85 88 90 29` and `preferences: 2` only: combine folded the entry copy `(set 75 (reg a3))` into `(set 90 (reg 75))` -> `(set 90 (reg a3))` (can_combine_p allows a hard-reg source on MIPS, no SMALL_REGISTER_CLASSES, combine.c:939-943), so the a3 event moved to b (pseudo 90, crosses the call, pref pruned) and 75 never saw a3. Same seat residual as K2. */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *slots;
    u8 *q;
    u8 *lnk;
    u8 *q2;
    u8 *lnk2;
    u8 *rec_a;
    u8 *rec_b;
    s32 sh;
    s32 sh2;
    s32 t;
    s32 t2;
    s32 i;
    s32 dist;
    s32 b;

    b = slot_b;
    if (slot_a == b) {
        return 0;
    }

    slots = *(u8 **)(ctx + 0xC);
    if (*(s32 *)((slot_a << 6) + (s32)slots + 0x18) >= 0) {
        if (*(s32 *)((b << 6) + (s32)slots + 0x18) >= 0) {
            return 0;
        }
    }

    sh = slot_a << 6;
    i = 0;
    q = *(u8 **)(ctx + 0xC);
    t = sh + (s32)q;
    t = *(s32 *)(t + 0x1C);
    if (i < t) {
        slot_b = (s32)q;
        q = *(u8 **)(ctx + 0x10);
        lnk = q;
        q = (u8 *)(sh + slot_b);
        do {
            if (*(u16 *)((*(u8 *)(q + i + 0x24) << 4) +
                         (s32)lnk + 0x4) == b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(q + 0x1C));
    }

    sh2 = slot_a << 6;
    i = 0;
    q2 = *(u8 **)(ctx + 0xC);
    t2 = sh2 + (s32)q2;
    t2 = *(s32 *)(t2 + 0x20);
    if (i < t2) {
        slot_b = (s32)q2;
        q2 = *(u8 **)(ctx + 0x10);
        lnk2 = q2;
        q2 = (u8 *)(sh2 + slot_b);
        do {
            if (*(s16 *)((*(u8 *)(q2 + i + 0x2C) << 4) +
                         (s32)lnk2 + 0x6) == b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(q2 + 0x20));
    }

    dist = math_Distance3D((s32 *)(*(u8 **)(ctx + 0xC) + (slot_a << 6)),
                           (s32 *)(*(u8 **)(ctx + 0xC) + (b << 6)));
    link = *(u8 **)(ctx + 0x10) + (*(s16 *)(ctx + 0x6) << 4);
    *(s32 *)(link + 0x0) = dist;
    *(s32 *)(link + 0x8) = dist * 3;
    *(s32 *)(link + 0xC) = arg1;
    *(s32 *)(link + 0x4) = (slot_a << 16) | b;

    rec_a = (u8 *)((slot_a << 6) + (s32) * (u8 **)(ctx + 0xC));
    i = *(s32 *)(rec_a + 0x1C);
    *(s32 *)(rec_a + 0x1C) = i + 1;
    *(u8 *)(rec_a + i + 0x24) = *(u16 *)(ctx + 0x6);

    rec_b = (u8 *)((b << 6) + (s32) * (u8 **)(ctx + 0xC));
    i = *(s32 *)(rec_b + 0x20);
    *(s32 *)(rec_b + 0x20) = i + 1;
    *(u8 *)(rec_b + i + 0x2C) = *(u16 *)(ctx + 0x6);

    *(s16 *)(ctx + 0x6) = *(u16 *)(ctx + 0x6) + 1;
    return 1;
}
