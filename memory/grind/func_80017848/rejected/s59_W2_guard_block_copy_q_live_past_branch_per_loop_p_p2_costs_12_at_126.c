/* REJECTED s59 (2026-09-15): cell W2 = 12 at 126/127. W1 with a distinct copy variable per loop (p, p2). Loop 1: copy survives, dest = a2 (conflicts v0, v1, a0, a1; lnk allocated AFTER p so a2 is free for p and lnk lands in a3): `addu a2,a0,zero` in the blez delay slot, `lw a3,16(s2)`, `addu a0,a1,a2`; sh stays a1. Loop 2: q has no use after loop 2's guard, so q's REG_DEAD sits before the branch, optimize_reg_copy_1 folds the copy (`addu a0,a1,a0`, 126 insns). Shows the copy-dest seat is decided by allocation ORDER once v0/a0 conflict: a3 needs lnk (and sh) allocated before p. */
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
    sh = slot_a << 6;
    i = 0;
    p = q;
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
