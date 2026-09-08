/* REJECTED (s22, 2026-09-07, structural) -- s22-three-invariant-holders-merge-one-move-score13.c
 *
 * ATTACK: the loop.c threshold sub-axis.  move_movables decays `threshold` by 3 for every
 * movable it actually moves (tools/gcc-2.7.2/loop.c:1904).  Starting threshold is 122 and the
 * four dispatch constants are evaluated at 119 against insn_count 91, so ten EXTRA moves ahead
 * of them (threshold 89 < 91) would make all four "not desirable" -- the exact result the target
 * shows.  This form asks whether duplicating the one invariant holder the function owns
 * (`lim = 0xC8`) into three distinct locals, one per 0xC8 store site, buys three decays.
 *
 * MEASURED on HEAD 121a39b5: score 13, build_insns 93 -- identical to the baseline.
 * The .loop dump (tmp/grind/func_8007526C/s22/loop-a-three-holders.txt) is the real result:
 *   Loop from 14 to 266: 92 real insns.
 *   Insn 19: regno 75 (life 109), move-insn savings 2  moved to 274
 *   Insn 25: regno 77 (life 45), done move-insn matches 19
 *   Insn 232/238/244/247: regno 126/128/129/130 (life 1) ... moved
 * Three holders collapse to ONE moved insn: cse1 deleted one of them outright and
 * combine_movables matched the second to the first (tools/gcc-2.7.2/loop.c:1283 accumulates
 * `m->savings += m1->savings`, which is why regno 75 now reports savings 2, and marks the match
 * `done` so move_movables' `if (!m->done ...)` guard at loop.c:1585 skips it entirely).  A
 * skipped movable never reaches the `threshold -= 3` at loop.c:1904.  So threshold decay counts
 * DISTINCT INVARIANT VALUES, not holder variables, and duplicating a holder is decay-neutral.
 */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim1;
    s32 lim2;
    s32 lim3;

    base = D_800A36A0;
    i = 0;
    do {
        lim1 = 0xC8;
        lim2 = 0xC8;
        lim3 = 0xC8;
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
                *(u16 *)(p + 0xC) = lim1;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
            }
            break;
        case 3:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                *(u16 *)(p + 8) = lim2;
                *(u16 *)(p + 0xC) = lim3;
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
