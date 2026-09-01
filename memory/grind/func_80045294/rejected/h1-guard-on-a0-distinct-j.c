/* REJECTED s49: score 38, build_insns 81 vs target 83.
 *
 * Form: the one untried COMBINATION of s48's two dead axes — H1 chassis, the
 * second loop's guard spelled on a0 (which gave nrefs_flow(72) = 6 here), and a
 * DISTINCT counter j for loop 2.  The point was cse.c:842-857: with loop 2
 * counted by j, i's last reference falls back inside loop 1, so a0's last
 * reference (the guard) postdates it, prong (2) of make_regs_eqv goes FALSE,
 * qty_first_reg stays reg 72 and block 0's ashift should keep a0 as its operand.
 *
 * MEASURED: build_insns 81, i.e. the s48 deletion reproduces unchanged — with a
 * distinct loop-2 counter the `lw %gp_rel(D_800A33AC)` that target keeps INSIDE
 * loop 2 at 0x8004538C hoists out of the loop and three instructions vanish.
 * The guard spelling does not affect that hoist.  This closes the last
 * combination in the s48 coupling argument: keeping a0 canonical for the shift
 * requires shortening i, and shortening i deletes instructions the target has.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 i = a0;
    s32 v1 = a0 << 4;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;

    if (i < count) {
        do {
            s32 val = *(s32 *)((u8 *)&D_800EED18 + v1);
            v1 += 0x10;
            i += 1;
            sum += val;
        } while (i < count);
    }

    if (sum != 0) {
        s32 *ptr;
        s32 idx;
        s32 j;

        gpu_DrawSync(0);
        func_800520B8(s4, s5, sum);

        if (a0 < D_800A33AC) {
            j = a0;
            v1 = j << 4;
            ptr = (s32 *)((u8 *)&D_800EED14 + v1);
            idx = v1;
            do {
                *ptr += a1;
                {
                    void (*fn)(s16, s32) = (void (*)(s16, s32)) *(s32 *)((u8 *)&D_800EED1C + idx);
                    if (fn != 0) {
                        fn(*(s16 *)((u8 *)&D_800EED10 + idx), a1);
                    }
                }
                ptr = (s32 *)((u8 *)ptr + 0x10);
                idx += 0x10;
                j += 1;
            } while (j < D_800A33AC);
        }
    }

    D_800A33A0 += a1;
    D_800A33A4 -= a1;
}
