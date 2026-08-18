/* candidate.c — func_80017848 (src/ings.c) — floor 16 (confirmed s1, 2026-08-18)
 * This is the CURRENT src/ings.c form (already in place in the tree). 125/127 insns.
 * Everything outside the two scan loops is byte-exact. Residual = 8 pts per scan loop:
 * mine caches the count in a1 (move a1,v0 preheader; slt v0,v1,a1 bound) and splits the
 * index add (addu v0,v1,a3 delay + addu v0,v0,a0 loop-top); target keeps ONE hoisted
 * base (addu a3,a0,zero copy of slots; addu a0,a1,a3) used by both the index
 * (addu v0,a0,v1) and a PER-ITERATION count reload (lw v0,0x1C(a0); nop; slt).
 * See memory/grind/func_80017848/hypotheses.md frontier before editing.
 */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *slots;
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
    i = 0;
    while (i < *(s32 *)((slot_a << 6) + (s32)slots + 0x1C)) {
        if (*(u16 *)((*(u8 *)(i + (slot_a << 6) + (s32)slots + 0x24) << 4) +
                     (s32) * (u8 **)(ctx + 0x10) + 0x4) == slot_b) {
            return 0;
        }
        i++;
    }
    slots = *(u8 **)(ctx + 0xC);
    i = 0;
    while (i < *(s32 *)((slot_a << 6) + (s32)slots + 0x20)) {
        if (*(s16 *)((*(u8 *)(i + (slot_a << 6) + (s32)slots + 0x2C) << 4) +
                     (s32) * (u8 **)(ctx + 0x10) + 0x6) == slot_b) {
            return 0;
        }
        i++;
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
