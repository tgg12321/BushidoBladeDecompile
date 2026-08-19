/* s14 chassis M - transcribed from m2c --valid-syntax output of the TARGET asm
 * (tools/m2c/m2c.py).  Structurally alien to V1/symmetric: rotated loops
 * (increment at top, `> 0` preheader guards), a WALKING element pointer
 * (elem = base + i) instead of base+i indexing, a single carried pointer local
 * `q` reused as the copy source in BOTH preheaders, sh recomputed at loop 1's
 * exit, and a separate counter for loop 2. */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *lnk;
    u8 *slots;
    u8 *p;
    u8 *q;
    u8 *base;
    u8 *elem;
    u8 *rec_a;
    u8 *rec_b;
    s32 sh;
    s32 i;
    s32 j;
    s32 dist;

    if (slot_a == slot_b) {
        return 0;
    }
    slots = *(u8 **)(ctx + 0xC);
    if (*(s32 *)((slot_a << 6) + (s32)slots + 0x18) >= 0 &&
        *(s32 *)((slot_b << 6) + (s32)slots + 0x18) >= 0) {
        return 0;
    }

    p = *(u8 **)(ctx + 0xC);
    sh = slot_a << 6;
    i = 0;
    if (*(s32 *)(sh + (s32)p + 0x1C) > 0) {
        q = p;
        lnk = *(u8 **)(ctx + 0x10);
        base = (u8 *)(sh + (s32)q);
        elem = base;
        do {
            i++;
            if (*(u16 *)((*(u8 *)(elem + 0x24) << 4) + (s32)lnk + 0x4) == slot_b) {
                return 0;
            }
            elem = base + i;
        } while (i < *(s32 *)(base + 0x1C));
        sh = slot_a << 6;
    }

    j = 0;
    if (*(s32 *)(sh + (s32) * (u8 **)(ctx + 0xC) + 0x20) > 0) {
        q = *(u8 **)(ctx + 0xC);
        lnk = *(u8 **)(ctx + 0x10);
        base = (u8 *)(sh + (s32)q);
        elem = base;
        do {
            j++;
            if (*(s16 *)((*(u8 *)(elem + 0x2C) << 4) + (s32)lnk + 0x6) == slot_b) {
                return 0;
            }
            elem = base + j;
        } while (j < *(s32 *)(base + 0x20));
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
