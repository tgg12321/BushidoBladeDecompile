/* REJECTED (s42, 2026-09-06): s42 cell U4 = the NATURAL SYMMETRIC CHASSIS: no exit tail, unconditional
 * `p = *(u8 **)(ctx + 0xC); i = 0; sh2 = slot_a << 6; t2 = sh2 + p;
 * t2 = *(t2 + 0x20); if (i < t2)` before loop 2, explicit `q = p;` +
 * `base = sh2 + q` in loop 2's preheader (U4b: fresh read for base2, identical).
 * 6 at 127/125 on the HEAD chassis. The normalised diff against target is
 * EXACTLY: both `addu a3,a0,zero` preheader copies absent (cse2 insns 83 and
 * 162 set reg/v 80 = q; both gone in .combine), the two base adds reading a0
 * instead of a3, and loop 2's links seat a1 instead of a2 (a consequence: with
 * the copy gone sh dies at the base add before the links load). Tail geometry
 * (lw a0,12(s2); sll a1,s4,6 on the fall-through only; blez landing on the
 * guard add) matches target WITHOUT any p = q device, via reorg.c's
 * redundant-insn redirect. Same body as s34's candidate_alt_join_shape_6.c
 * modulo declaration order. This is the chassis on which the two copies must
 * be made to survive combine AND be live across their loops (seat a3).
 */
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
            if (*(u16 *)((*(u8 *)(base + i + 0x24) << 4) +
                         (s32)lnk + 0x4) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(base + 0x1C));
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
