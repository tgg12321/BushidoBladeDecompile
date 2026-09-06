/* REJECTED - s44 M7 - fold sourced from the loop's lbu value (idx = lbu; y = idx & 0x100; z = q & y), a CHEAT by policy.
 * 35 at 127/126: the named idx spans the beq block boundary, becomes a global pseudo with top priority (v1) and
 * rotates every loop seat; copies survive (`addu a2,v1,zero` / `addu a1,a1,a2`). Unusable as an instrument. */
/* s44 cell M7 - MECHANISM INSTRUMENT ONLY (dead algebra by construction: idx is a zero-extended byte so
 * idx & 0x100 is 0, z = q & 0 is 0, base + z is base; a CHEAT by policy, never a candidate). Fold source is
 * the loop's own lbu value (nonzero_bits 0xFF) so no pre-existing pseudo's live range is stretched; distinct
 * copy destination per loop (q / q2). Question: copy survival + does the copy destination seat in a3? */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *lnk;
    u8 *slots;
    u8 *p;
    u8 *q;
    u8 *q2;
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
    s32 y;
    s32 z;

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
            if (*(u16 *)((idx << 4) + (s32)lnk + 0x4) == slot_b) {
                return 0;
            }
            i++;
            y = idx & 0x100;
            z = (s32)q & y;
        } while (i < *(s32 *)(base + z + 0x1C));
    }

    p = *(u8 **)(ctx + 0xC);
    i = 0;
    sh2 = slot_a << 6;
    t2 = sh2 + (s32)p;
    t2 = *(s32 *)(t2 + 0x20);
    if (i < t2) {
        q2 = p;
        base = (u8 *)(sh2 + (s32)q2);
        do {
            idx = *(u8 *)(base + i + 0x2C);
            if (*(s16 *)((idx << 4) + (s32) * (u8 **)(ctx + 0x10) + 0x6) == slot_b) {
                return 0;
            }
            i++;
            y = idx & 0x100;
            z = (s32)q2 & y;
        } while (i < *(s32 *)(base + z + 0x20));
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
