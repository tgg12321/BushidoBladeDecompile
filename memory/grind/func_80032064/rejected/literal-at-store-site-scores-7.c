/* REJECTED — sandbox --disable all == 7 (target_insns 97, build_insns 97).
 *
 * This is the JUDGE-PRESCRIBED probe from the s1 ledger: drop the constant-
 * holder local entirely and write the literal at the store site.
 *
 * Measured failure (objdump of the sandbox .o, tmp/grind/func_80032064/s2/
 * literal_probe_full.txt): GCC emits
 *
 *     mult  v0,a1
 *     li    v0,-200        <-- INSIDE the mult window, in .L800320C4's BB
 *     sw    v0,32(s0)
 *     mflo  t0
 *
 * where the target has `addiu $t0,$zero,-0xC8` at instruction 5, in the ENTRY
 * basic block, and only the `sw $t0,0x20($s0)` inside the mult window.
 *
 * Mechanism: GCC 2.7.2 sched.c is basic-block-local (no cross-BB code motion).
 * The store's BB begins at .L800320C4, separated from the entry BB by the
 * .L8003208C search loop and two branches. A literal expands adjacent to its
 * use, inside that BB, and no pass can move it into the entry BB. The constant
 * therefore cannot reach instruction 5 unless the C declares a live pseudo
 * initialized at function entry.
 *
 * The negative is banked: the literal form is DEAD.
 */
u8 *func_80032064(u8 *src, s32 type) {
    s32 mul;
    s32 i;
    u8 *ptr;
    u8 *s0;
    s16 sp_area[2];

    mul = 0x50;
    i = 0;
    ptr = &D_80104E88;

    for (i = 0; i < 4; i++) {
        s0 = ptr;
        if (*s0 == 0) break;
        ptr = s0 + 0x2C;
    }
    if (i == 4) return 0;

    /* ... unchanged prologue stores ... */

    *(s32 *)(s0 + 0x1C) = ((s32)*(&Judge + (*(u16 *)(src + 0x1CA) & 0xFFF)) * mul) >> 12;
    *(s32 *)(s0 + 0x20) = -0xC8;   /* <-- THE PROBE: literal, no holder local */
    *(s32 *)(s0 + 0x24) = ((s32)*(&Judge + ((*(s16 *)(src + 0x1CA) + 0x400) & 0xFFF)) * mul) >> 12;

    /* ... unchanged tail ... */
    return s0;
}
