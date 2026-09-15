/* REJECTED s59 (2026-09-15): cell W1 = 16 at 127/127. K2 chassis with the copy `p = q` moved into the GUARD block and q kept LIVE PAST the branch (loop 2's guard reads q and sh; loop 1's exit path re-assigns `q = *(u8 **)(ctx + 0xC); sh = slot_a << 6;` inside the if after the do-while), one copy variable p for both loops. RESULT: both preheader copies SURVIVE every pass (no REG_DEAD for q before the JUMP_INSN, so optimize_reg_copy_1's scan stops at the branch, local-alloc.c:721-725) and the copy dest now CONFLICTS with the guard's v0 temporaries and with q (a0) - the seat moves off v0 for the first time: loop-1 copy dest = a1, lnk = a3, sh = a2 (p allocated before sh and lnk: 4 refs / short range = high priority). But the copy is emitted in the blez DELAY SLOT (it is the last insn of the guard block; reorg fill_simple_delay_slots takes it) and `i = 0` is scheduled above the guard add - the target's copy sits AFTER the delay slot, i.e. in the preheader block. Also the sh re-assignment duplicates `sll a2,s4,6` into the early-return load-delay slots. */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *slots;
    u8 *p;
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
    p = q;
    t = sh + (s32)q;
    t = *(s32 *)(t + 0x20);
    if (i < t) {
        q = *(u8 **)(ctx + 0x10);
        lnk = q;
        q = (u8 *)(sh + (s32)p);
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
