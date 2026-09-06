/* REJECTED - s44 M2 - instrument attempt (idx = lbu; if (idx >> 8) return (s32)q;), a CHEAT by policy. 45 at 127/135 (M2a
 * loop-1-only 38 at 131): combine does NOT fold `(ashiftrt idx 8)` to 0 through the jump (`sra v0,v1,8; bne`
 * survive), so the reader stays real. Not an instrument; recorded so the shift spelling is not retried. */
/* s44 cell M2 - MECHANISM INSTRUMENT ONLY (dead code by construction: idx is a u8 so idx >> 8 is 0;
 * a CHEAT by policy, never a candidate). Zero-extra-instruction variant of M1: the reader `return (s32)q`
 * sits in a branch whose condition combine folds via nonzero_bits(lbu) = 0xFF, so the reader is
 * unreachable after combine and deleted by jump2, but flow (before combine) saw q live across the loop. */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *lnk;
    u8 *slots;
    u8 *p;
    u8 *q;
    u8 *base;
    u8 *rec_a;
    u8 *rec_b;
    s32 sh;
    s32 sh2;
    s32 t;
    s32 t2;
    s32 i;
    s32 dist;
    s32 idx;

    if (slot_a == slot_b) {
        return 0;
    }

    slots = *(u8 **)(ctx + 0xC);
    if (*(s32 *)((slot_a << 6) + (s32)slots + 0x18) >= 0) {
        if (*(s32 *)((slot_b << 6) + (s32)slots + 0x18) >= 0) {
            return 0;
        }
    }

    p = *(u8 **)(ctx + 0xC);
    sh = slot_a << 6;
    i = 0;
    t = sh + (s32)p;
    t = *(s32 *)(t + 0x1C);
    if (i < t) {
        q = *(u8 **)(ctx + 0xC);
        lnk = *(u8 **)(ctx + 0x10);
        base = (u8 *)(sh + (s32)q);
        do {
            idx = *(u8 *)(base + i + 0x24);
            if (idx >> 8) {
                return (s32)q;
            }
            if (*(u16 *)((idx << 4) + (s32)lnk + 0x4) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(base + 0x1C));
    }

    p = *(u8 **)(ctx + 0xC);
    i = 0;
    sh2 = slot_a << 6;
    t2 = sh2 + (s32)p;
    t2 = *(s32 *)(t2 + 0x20);
    if (i < t2) {
        q = p;
        base = (u8 *)(sh2 + (s32)q);
        do {
            idx = *(u8 *)(base + i + 0x2C);
            if (idx >> 8) {
                return (s32)q;
            }
            if (*(s16 *)((idx << 4) + (s32) * (u8 **)(ctx + 0x10) + 0x6) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(base + 0x20));
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
