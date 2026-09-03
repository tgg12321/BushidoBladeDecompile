/* s53 REJECTED (score 1/83, byte-identical to its decl-split control K0). 'a0 = a0;' placed between the block-0 copy and the guard was intended to invalidate reg 72's cse quantity so that qty_first_reg falls back to i. The store never reaches RTL (expand folds a self-assignment of a pseudo to itself), so it cannot flip the canonical register; the residual is unchanged at idx 9. */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 i = a0;
    s32 v1;
    s32 s4;
    s32 count;
    s32 s5;

    a0 = a0;
    v1 = a0 << 4;
    s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    count = D_800A33AC;
    s5 = s4 + a1;

    if (i < count) {
        do {
            s32 val = *(s32 *)((u8 *)&D_800EED18 + v1);
            v1 += 0x10;
            i += 1;
            sum += val;
        } while (i < count);
    }

    if (sum != 0) {
        s32 idx;

        gpu_DrawSync(0);
        func_800520B8(s4, s5, sum);

        i = a0;
        if (i < D_800A33AC) {
            v1 = i << 4;
            a0 = (s32)((u8 *)&D_800EED14 + v1);
            idx = v1;
            do {
                *(s32 *)a0 += a1;
                {
                    void (*fn)(s16, s32) = (void (*)(s16, s32)) *(s32 *)((u8 *)&D_800EED1C + idx);
                    if (fn != 0) {
                        fn(*(s16 *)((u8 *)&D_800EED10 + idx), a1);
                    }
                }
                a0 += 0x10;
                idx += 0x10;
                i += 1;
            } while (i < D_800A33AC);
        }
    }

    D_800A33A0 += a1;
    D_800A33A4 -= a1;
}
