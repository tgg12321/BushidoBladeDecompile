/* REJECTED — func_80017848, s2.  score 35, frame 0x30 (vars=0).
 * The do-while + slots-copy + single-base form with an ORDINARY `if (count > 0)` entry
 * guard.  Its loops are target-correct (reload + single-base index) but the phantom-16
 * frame is lost, and the sp-offset cascade through the save/restore block costs ~19.
 * KILLS H1's central claim that a distinct guard/bound LVALUE is what produces the
 * phantom slots: guard lvalue is irrelevant.  What matters is the guard COMPARISON
 * OPERAND — only `i < count` (the loop's own induction variable) produces vars=16.
 * Whole family measured frame 0x30: guard `> 0`, `!= 0`, `>= 1`, `0 <`, named-count
 * local, `count - 1 != -1`, and bound-through-named-count.
 */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *slots;
    u8 *p;
    u8 *base;
    u8 *rec_a;
    u8 *rec_b;
    s32 i;
    s32 dist;

    if (slot_a == slot_b) {
        return 0;
    }
    if (*(s32 *)((slot_a << 6) + (s32) * (u8 **)(ctx + 0xC) + 0x18) >= 0) {
        if (*(s32 *)((slot_b << 6) + (s32) * (u8 **)(ctx + 0xC) + 0x18) >= 0) {
            return 0;
        }
    }

    slots = *(u8 **)(ctx + 0xC);
    if (*(s32 *)((slot_a << 6) + (s32)slots + 0x1C) > 0) {
        p = slots;
        base = (u8 *)((slot_a << 6) + (s32)p);
        i = 0;
        do {
            if (*(u16 *)((*(u8 *)(base + i + 0x24) << 4) +
                         (s32) * (u8 **)(ctx + 0x10) + 0x4) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(base + 0x1C));
    }
    slots = *(u8 **)(ctx + 0xC);
    if (*(s32 *)((slot_a << 6) + (s32)slots + 0x20) > 0) {
        p = slots;
        base = (u8 *)((slot_a << 6) + (s32)p);
        i = 0;
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
