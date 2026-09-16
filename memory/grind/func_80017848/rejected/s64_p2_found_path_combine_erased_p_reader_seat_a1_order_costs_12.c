/* REJECTED s64 - cell P2: K2 chassis + found-path exit `return ((s32)p << 4) & 0xF;` (loop 1 only), a combine-erased (simplify_and_const_int / nonzero_bits) reader of the copy dest p on the return-0 tail.
 * = 12 at 127/127.  The mechanism fires (p leaves v0, target's instruction stream reproduced exactly) but the seat is a1, not a3:
 * p's loop-spanning live range makes global.c allocate it before sh and lnk (sh -> a2, lnk -> a3), the identical residual to s44's M5 instrument (E-s44-4).
 * FAKE dead-algebra instrument, measurement only; never a candidate. */
/* [s64 instrument cell P2: frontier item 2 - found-path exit reader of p that combine erases (simplify_and_const_int via nonzero_bits); measurement-only, FAKE: dead algebra, mechanism: combine.c simplify_and_const_int, lever-exhaustion: hypotheses.md s44/s63] */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *slots;
    u8 *p;
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
    if (i < t) {
        p = q;
        q = *(u8 **)(ctx + 0x10);
        lnk = q;
        q = (u8 *)(sh + (s32)p);
        do {
            if (*(u16 *)((*(u8 *)(q + i + 0x24) << 4) +
                         (s32)lnk + 0x4) == slot_b) {
                return ((s32)p << 4) & 0xF; /* FAKE: dead algebra, mechanism: combine.c simplify_and_const_int, lever-exhaustion: hypotheses.md s44/s63 */
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
        p = q2;
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
