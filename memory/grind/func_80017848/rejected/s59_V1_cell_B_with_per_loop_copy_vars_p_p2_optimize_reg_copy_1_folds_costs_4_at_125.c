/* REJECTED s59 (2026-09-15): cell V1 = 4 at 125/127 (same 125-insn shape as s57 cell B). Cell B with a DISTINCT copy variable per loop (p for loop 1, p2 for loop 2) so that cse's canonical-register rule (cse.c:844-857: the new reg becomes canonical only if its last use is later than the old canonical's) keeps q canonical and the cse copy-swap at cse.c:7454 (which fires only when the copy DEST is the qty's first reg and the PREVIOUS insn sets the source) does not fire. Measured: the copy `(set p q)` now SURVIVES cse and combine (dumps s59/skel_cse_V4.txt, skel_combine_V4.txt: the add reads p), but local-alloc's optimize_reg_copy_1 (local-alloc.c:700-790) fires because q's REG_DEAD sits at the guard address insn BEFORE the JUMP_INSN where its forward scan stops: it rewrites the guard's use of q to p, q dies at the copy, p and q share a0 and the copy is deleted as a no-op move. */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *slots;
    u8 *p;
    u8 *p2;
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
    }

    sh2 = slot_a << 6;
    i = 0;
    q2 = *(u8 **)(ctx + 0xC);
    p2 = q2;
    t2 = sh2 + (s32)q2;
    t2 = *(s32 *)(t2 + 0x20);
    if (i < t2) {
        q2 = *(u8 **)(ctx + 0x10);
        lnk2 = q2;
        q2 = (u8 *)(sh2 + (s32)p2);
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
