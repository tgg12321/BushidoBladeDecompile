/* CANDIDATE — func_80084A7C, grind session 1 (recon), 2026-08-17.
 * Honest sandbox floor (--disable all): 26, build 146 insns vs target 145.
 *
 * Improvement over the inherited HEAD form: the eight flag-word expressions
 * are spelled `offset + *base_ptr + 0x98` (offset FIRST) instead of
 * `*base_ptr + offset + 0x98`.  That is score-NEUTRAL (26 -> 26) but it
 * removes a whole diff axis: every `addu` at those sites now has target's
 * operand order (`addu vX, <offset>, vX`), so the residual is a pure
 * register-naming difference rather than operand-order + register.  Keep it.
 * `shifted` was inlined into base_ptr (also 26; one fewer named local).
 *
 * REMAINING RESIDUAL (the whole of it):
 *   1. base_ptr lives in $a2 (target: $a3) and offset in $a3+$a1
 *      (target: $a2 throughout) -- a 2-register swap.
 *   2. ONE extra instruction: `move $a1, $a3` in the entry block, a failed
 *      coalesce between the *0xB0 multiply chain's final `sll` destination
 *      ($a3, a block-local quantity) and the `offset` global allocno ($a1).
 *      Target coalesces both into $a2, hence 145 vs our 146.
 * Everything else is byte-identical in shape.
 */
}
/* kengo:MED  |  sa_tan4/saTan4GaugeInit  |  66i */
void func_80084A7C(s16 a0, s16 a1) {
    s32 *base_ptr = (s32 *)((u8 *)&D_80106F28 + ((a0 << 16) >> 14));
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
