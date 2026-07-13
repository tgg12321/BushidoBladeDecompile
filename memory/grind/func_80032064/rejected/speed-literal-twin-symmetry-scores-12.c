/* REJECTED — sandbox --disable all == 12, build_insns 95 vs target 97.
 *
 * THE SYMMETRY PROBE — the most important measurement of session 2.
 *
 * The audit flagged `sw_val = -0xC8` (single-use constant-holder local) as a
 * "scheduling coercion" and did NOT flag its twin `mul = 0x50` (same cluster,
 * same shape, two uses). This probe inlines the UNFLAGGED twin as a literal,
 * holding the flagged one constant:
 *
 *     *(s32 *)(s0 + 0x1C) = (... * 0x50) >> 12;
 *     *(s32 *)(s0 + 0x24) = (... * 0x50) >> 12;
 *
 * Result: score 12, and GCC emits TWO FEWER instructions than target (95 vs
 * 97). The unflagged `mul` local is byte-load-bearing in exactly the same way
 * and for exactly the same reason as the flagged `sw_val`: without the local,
 * the 0x50 materialization cannot reach the entry basic block (asm line 5,
 * `addiu $a1,$zero,0x50`), because sched.c does not move code across BBs.
 *
 * Consequence for the ruling: `mul`/`speed` and `sw_val`/`vel_y` are THE SAME
 * CONSTRUCT. Any policy that calls one a scheduling coercion must call the
 * other one too — and `i = 0` (score 2 when dropped) and `ptr = &D_80104E88`
 * as well. All four are ordinary locals initialized at the top of the
 * function. The distinguishing feature the audit relied on (single use) is not
 * what makes the construct load-bearing; being a MUTABLE PSEUDO initialized at
 * function entry is.
 */
u8 *func_80032064(u8 *src, s32 type) {
    s32 sw_val;
    s32 i;
    u8 *ptr;
    u8 *s0;
    s16 sp_area[2];

    sw_val = -0xC8;
    i = 0;
    ptr = &D_80104E88;

    for (i = 0; i < 4; i++) {
        s0 = ptr;
        if (*s0 == 0) break;
        ptr = s0 + 0x2C;
    }
    if (i == 4) return 0;

    /* ... unchanged ... */

    /* THE PROBE: the unflagged twin, inlined as a literal at both use sites */
    *(s32 *)(s0 + 0x1C) = ((s32)*(&Judge + (*(u16 *)(src + 0x1CA) & 0xFFF)) * 0x50) >> 12;
    *(s32 *)(s0 + 0x20) = sw_val;
    *(s32 *)(s0 + 0x24) = ((s32)*(&Judge + ((*(s16 *)(src + 0x1CA) + 0x400) & 0xFFF)) * 0x50) >> 12;

    return s0;
}
