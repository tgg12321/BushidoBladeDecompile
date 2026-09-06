/* REJECTED - s44 M1 - MECHANISM INSTRUMENT, a CHEAT by policy (dead algebra m = slot_b & 1, w = m >> 1, y = i & w,
 * z = q & y consumed in the exit-test address). NEVER a candidate. 14 at 127/128. PROVES: a loop-body read of
 * the copy destination that survives cse1/loop/cse2/flow and is folded to a constant by COMBINE (nonzero_bits ->
 * simplify_and_const_int) keeps the preheader copy alive (`addu a1,a0,zero` then `addu a0,a2,a1`) with ZERO
 * reader bytes in the loop; global.c then allocates q as live across the loop because flow's
 * basic_block_live_at_start is never recomputed after combine. Seats perturbed by the extra pseudos m/w. */
/* s44 cell M1 - MECHANISM INSTRUMENT ONLY (dead-algebra reader; a CHEAT by policy, never a candidate).
 * Question: when a loop-body use of the copy destination q survives cse/loop/flow but is folded to a
 * constant by combine (nonzero_bits: m = slot_b & 1 -> w = m >> 1 == 0 -> y = i & w == 0 -> z = q & y == 0),
 * does global.c still allocate q as live across the loop (flow's basic_block_live_at_start is not recomputed
 * after combine) and does the preheader copy survive combine (q not dead at the base add)? */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *lnk;
    u8 *slots;
    u8 *p;
    u8 *q;
    u8 *base;
    u8 *rec_a;
    u8 *rec_b;
    s32 sh;
    s32 sh2;
    s32 t;
    s32 t2;
    s32 i;
    s32 dist;
    s32 m;
    s32 w;
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

    m = slot_b & 1;
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
            if (*(u16 *)((*(u8 *)(base + i + 0x24) << 4) +
                         (s32)lnk + 0x4) == slot_b) {
                return 0;
            }
            i++;
            w = m >> 1;
            y = i & w;
            z = (s32)q & y;
        } while (i < *(s32 *)(base + z + 0x1C));
    }

    p = *(u8 **)(ctx + 0xC);
    i = 0;
    sh2 = slot_a << 6;
    t2 = sh2 + (s32)p;
    t2 = *(s32 *)(t2 + 0x20);
    if (i < t2) {
        q = p;
        base = (u8 *)(sh2 + (s32)q);
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
