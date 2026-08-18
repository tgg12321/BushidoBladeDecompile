/* ALT candidate — func_80017848, s2 (2026-08-18).  sandbox --disable all = 17
 * (floor stays 16 = candidate.c), BUT structurally much closer to target than the
 * floor-16 form: BOTH scan loops are byte-exact here — target's per-iteration bound
 * reload (lw v0,0x1C(a0); nop; slt v0,v1,v0), its single hoisted base and its one-addu
 * index (addu v0,a0,v1) all reproduce, and the frame is 0x40 / vars=16.
 * The ENTIRE residual is now the 2-insn preheader shape per loop: target re-loads
 * ctx+0xC (lw a0,12(s2)) and keeps an uncoalesced slots copy (move a3,a0) feeding a
 * recomputed base (addu a0,a1,a3); this form reuses the top-guard's slots pseudo (v1).
 * The two enabling levers, both measured this session:
 *   (1) do-while (source-level) gives the per-iteration reload;
 *   (2) the entry guard MUST be spelled `i = 0; if (i < count)` — comparing the loop's
 *       own induction variable — to keep the phantom-16 frame.  Every other guard
 *       spelling (> 0, != 0, >= 1, 0 <, named count, count-1 != -1) drops to frame 0x30.
 * Next session: start HERE, not from candidate.c.  See hypotheses.md s2 frontier.
 */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *slots;
    u8 *p;
    u8 *base;
    u8 *rec_a;
    u8 *rec_b;
    s32 i;
    s32 n;
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
    if (i < *(s32 *)((slot_a << 6) + (s32)slots + 0x1C)) {
        p = slots;
        base = (u8 *)((slot_a << 6) + (s32)p);
        do {
            if (*(u16 *)((*(u8 *)(base + i + 0x24) << 4) +
                         (s32) * (u8 **)(ctx + 0x10) + 0x4) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(base + 0x1C));
    }
    slots = *(u8 **)(ctx + 0xC);
    i = 0;
    if (i < *(s32 *)((slot_a << 6) + (s32)slots + 0x20)) {
        p = slots;
        base = (u8 *)((slot_a << 6) + (s32)p);
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
