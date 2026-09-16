/* s61 J1 - ledger frontier item 1 (s60): per-loop copy dest p with a post-loop
 * reader `x = p` consumed by loop 2's guard, x pre-defined as `x = q` before
 * loop 1's guard so it is initialised on the blez path. Prediction (source
 * reading): x must sit in a0 on the taken path (guard 2 reads it there) and in
 * p's seat on the fall-through, so either the guard-block copy `x = q` prints
 * or the tail copy prints; the target has a fresh `lw a0,0xC(s2)` there. */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *slots;
    u8 *p;
    u8 *q;
    u8 *x;
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

    if (slot_a == slot_b) {
        return 0;
    }

    slots = *(u8 **)(ctx + 0xC);
    if (*(s32 *)((slot_a << 6) + (s32)slots + 0x18) >= 0) {
        if (*(s32 *)((slot_b << 6) + (s32)slots + 0x18) >= 0) {
            return 0;
        }
    }

    sh = slot_a << 6;
    i = 0;
    q = *(u8 **)(ctx + 0xC);
    t = sh + (s32)q;
    t = *(s32 *)(t + 0x1C);
    x = q;
    if (i < t) {
        p = q;
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
        x = p;
    }

    sh2 = slot_a << 6;
    i = 0;
    t2 = sh2 + (s32)x;
    t2 = *(s32 *)(t2 + 0x20);
    if (i < t2) {
        p = x;
        q2 = *(u8 **)(ctx + 0x10);
        lnk2 = q2;
        q2 = (u8 *)(sh2 + (s32)p);
        do {
            if (*(s16 *)((*(u8 *)(q2 + i + 0x2C) << 4) +
                         (s32)lnk2 + 0x6) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(q2 + 0x20));
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
