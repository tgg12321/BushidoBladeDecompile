/* candidate for func_8007526C (src/text1b.c) -- s1 (2026-09-07), ORDINARY C, real loop.
 *
 * MEASURED THIS SESSION on HEAD 73e786dd with the current unmodified build configuration:
 *   `sandbox func_8007526C --disable all` -> score 13, build_insns 93, target_insns 91.
 *
 * This is the best form that is BOTH legal and free of the banned goto-loop spelling
 * (state.json banned_constructs[0]; the floor-1 goto body is parked in
 * rejected/goto-loop-banned-layer2-fail-score1.c).  The entire 13-point residual is one
 * loop.c/move_movables decision: the four switch-comparison constants 1/2/3/4 are hoisted
 * into the loop pre-header here, while asm/funcs/func_8007526C.s materialises them inside
 * the loop (lines 8, 12, 18, 20) and hoists only the 0xC8 movable (line 3).
 *
 * Measured mechanism (tmp/grind/func_8007526C/dumps/text1b.loop, s1):
 *   "Loop from 14 to 263: 92 real insns."
 *   Insn 229: regno 126 (life 1), move-insn savings 1  moved to 271     <- the constant 2
 *   Insn 235/241/244 ... same shape                                     <- 1, 3, 4
 *   Insn 76: regno 92 (life 3), move-insn savings 2  moved to 279       <- the 0xC8
 * move_movables moves a movable when (threshold * savings * lifetime) >= insn_count
 * (tools/gcc-2.7.2/loop.c:1631) with threshold = (loop_has_call ? 1 : 2) * (1 +
 * n_non_fixed_regs) (loop.c:532).  Hard-float cc1 has n_non_fixed_regs == 60, so
 * threshold == 122 and 122*1*1 >= 92 moves each constant.  The target's shape needs the
 * product to fall below insn_count for savings==1, lifetime==1 movables.
 */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;

    base = D_800A36A0;
    i = 0;
    do {
        p = base + i * 2;
        switch (*(u8 *)(p + 0x10)) {
        case 1:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) + 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0xC8;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
            }
            break;
        case 3:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                *(u16 *)(p + 8) = 0xC8;
                *(u16 *)(p + 0xC) = 0xC8;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
            }
            break;
        case 2:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        case 4:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) - 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        }
        i++;
    } while (i < 2);
}
