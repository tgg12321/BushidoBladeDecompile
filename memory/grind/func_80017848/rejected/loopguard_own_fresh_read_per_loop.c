/* REJECTED - s3.  score 17.  Gives each loop entry guard its OWN `p = *(u8**)(ctx+0xC)`
 * read immediately before `i = 0` (with the base then reading fresh again), i.e. the
 * three-live-reads shape target appears to have.  A third read reaching the preheader
 * COSTS 3 pts rather than producing target's `move a3,a0` copy: the extra pseudo is
 * coalesced and the hoisted-slots guard form (14) is strictly better. */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *slots;
    u8 *p;
    u8 *base;
    u8 *rec_a;
    u8 *rec_b;
    u8 *ga;
    u8 *gb;
    u8 *g;
    s32 gi;
    u8 *q;
    s32 i;
    s32 n;
    s32 dist;

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
    i = 0;
    if (i < *(s32 *)((s32)p + (slot_a << 6) + 0x1C)) {
        q = *(u8 **)(ctx + 0xC);
        base = (u8 *)((slot_a << 6) + (s32)q);
        do {
            if (*(u16 *)((*(u8 *)(base + i + 0x24) << 4) +
                         (s32) * (u8 **)(ctx + 0x10) + 0x4) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(base + 0x1C));
    }
    p = *(u8 **)(ctx + 0xC);
    i = 0;
    if (i < *(s32 *)((s32)p + (slot_a << 6) + 0x20)) {
        q = *(u8 **)(ctx + 0xC);
        base = (u8 *)((slot_a << 6) + (s32)q);
        do {
            if (*(s16 *)((*(u8 *)(base + i + 0x2C) << 4) +
                         (s32) * (u8 **)(ctx + 0x10) + 0x6) == slot_b) {
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

