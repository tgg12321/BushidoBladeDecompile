/* NOT A CHEAT — this is the best form of func_80084A7C that contains NO
 * unsanctioned construct.  It is banked here (rather than as candidate.c)
 * only because it does not reach distance 0.
 *
 * Honest sandbox floor (`--disable all`): 24, build_insns 146 vs target 145.
 * (Session 2, 2026-08-17.  Session 1's floor was 26; the 2-point gain is the
 * sibling-idiom entry spelling, see below.)
 *
 * It differs from candidate.c in exactly one character sequence: the offset is
 * the plain `(s16)a1 * 0xB0` instead of the negation-pair `-((s16)a1 * -0xB0)`.
 * With the plain spelling, global-alloc puts the `offset` allocno in $a1 (it
 * has no conflict with hard reg 5 and no copy preference), which costs the
 * $a2/$a3 swap across ~18 instructions plus the 146th instruction
 * `move $a1,$a3` (a failed coalesce with the multiply chain's final `sll`).
 *
 * Its entry block IS already target-correct in schedule: the sibling idiom
 * (`s32 shifted` + `s32 *addr`, copied verbatim from the matched neighbour
 * spu_SetMotionState at src/main.c:303) puts the D_80106F28 address load
 * between `sll v0,a0,16` and `sra v0,v0,14` exactly as target does.
 *
 * If the owner refuses the negation lever, THIS is the resume point: the
 * remaining problem is precisely "make the offset allocno conflict with $a1
 * (or prefer $a2) without an arithmetic no-op".
 */
/* kengo:MED  |  sa_tan4/saTan4GaugeInit  |  66i */
void func_80084A7C(s16 a0, s16 a1) {
    s32 shifted = a0 << 16;
    s32 *addr = (s32 *)&D_80106F28;
    s32 *base_ptr = (s32 *)((u8 *)addr + (shifted >> 14));
    s32 offset = (s16)a1 * 0xB0;
    u8 *base = (u8 *)(*base_ptr + offset);
    s32 val;
    u32 threshold;

    val = base[0x21] + 1;
    threshold = base[0x20];
    base[0x21] = val;

    if (threshold == 0) {
        *(s32 *)(base + 0x88) = 0;
        base[0x1C] = 0;
        *(s32 *)(base + 0x90) = 0;
        if (*(s32 *)(offset + *base_ptr + 0x98) & 0x400) {
            *(s32 *)base = *(s32 *)(base + 0xC);
        } else {
            *(s32 *)base = *(s32 *)(base + 4);
        }
        return;
    }

    if ((u8)val < threshold) {
        *(s32 *)(base + 0x88) = 0;
        base[0x1C] = 0;
        *(s32 *)(base + 0x90) = 0;
        if (*(s32 *)(offset + *base_ptr + 0x98) & 0x400) {
            *(s32 *)base = *(s32 *)(base + 0xC);
            *(s32 *)(base + 8) = *(s32 *)(base + 0xC);
        } else {
            *(s32 *)base = *(s32 *)(base + 4);
            *(s32 *)(base + 8) = *(s32 *)(base + 4);
        }
        return;
    }

    *(s32 *)(offset + *base_ptr + 0x98) &= ~1;
    *(s32 *)(offset + *base_ptr + 0x98) &= ~8;
    *(s32 *)(offset + *base_ptr + 0x98) &= ~2;
    *(s32 *)(offset + *base_ptr + 0x98) |= 0x200;
    *(s32 *)(offset + *base_ptr + 0x98) |= 4;
    base[0x14] = 0;

    if (*(s32 *)(offset + *base_ptr + 0x98) & 0x400) {
        *(s32 *)(base + 8) = *(s32 *)(base + 0xC);
    } else {
        *(s32 *)(base + 8) = *(s32 *)(base + 4);
    }

    if (base[0x22] != 0xFF) {
        base[0x14] = 0;
        spu_ResetMotionEntry(base[0x22], base[0x23]);
        spu_NotifyChannel((s16)(a0 | (a1 << 8)));
    }
    spu_NotifyChannel((s16)(a0 | (a1 << 8)));
    *(s32 *)(base + 0x90) = *(s16 *)(base + 0x54);
}
