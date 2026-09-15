/* REJECTED s59 (2026-09-15): cell W4 = 12 at 126/127, byte-identical to W2. W2 with the copy placed immediately after the ctx+0xC load (before `sh = slot_a << 6; i = 0;`) to lengthen p's live range and drop its priority below lnk's. No change: loop-1 copy dest still a2, lnk a3; the cse copy-swap (cse.c:7454) did not fire because q stays the qty's canonical reg (its last use is later than p's). */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *slots;
    u8 *p;
    u8 *p2;
    u8 *q;
    u8 *lnk;
    u8 *rec_a;
    u8 *rec_b;
    s32 sh;
    s32 t;
    s32 i;
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

    q = *(u8 **)(ctx + 0xC);
    p = q;
    sh = slot_a << 6;
    i = 0;
    t = sh + (s32)q;
    t = *(s32 *)(t + 0x1C);
    if (i < t) {
        q = *(u8 **)(ctx + 0x10);
        lnk = q;
        q = (u8 *)(sh + (s32)p);
        do {
            if (*(u16 *)((*(u8 *)(q + i + 0x24) << 4) +
                         (s32)lnk + 0x4) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(q + 0x1C));
        q = *(u8 **)(ctx + 0xC);
        sh = slot_a << 6;
    }

    i = 0;
    p2 = q;
    t = sh + (s32)q;
    t = *(s32 *)(t + 0x20);
    if (i < t) {
        q = *(u8 **)(ctx + 0x10);
        lnk = q;
        q = (u8 *)(sh + (s32)p2);
        do {
            if (*(s16 *)((*(u8 *)(q + i + 0x2C) << 4) +
                         (s32)lnk + 0x6) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(q + 0x20));
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
