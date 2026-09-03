void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 i = a0;
    s32 v1 = a0 << 4;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;

    a0 = i;

    if (i < count) {
        do {
            s32 val = *(s32 *)((u8 *)&D_800EED18 + v1);
            v1 += 0x10;
            i += 1;
            sum += val;
        } while (i < count);
    }

    if (sum != 0) {
        s32 j;

        gpu_DrawSync(0);
        func_800520B8(s4, s5, sum);

        j = a0;
        if (j < D_800A33AC) {
            v1 = j << 4;
            a0 = (s32)((u8 *)&D_800EED14 + v1);
            i = v1;
            do {
                *(s32 *)a0 += a1;
                {
                    void (*fn)(s16, s32) = (void (*)(s16, s32)) *(s32 *)((u8 *)&D_800EED1C + i);
                    if (fn != 0) {
                        fn(*(s16 *)((u8 *)&D_800EED10 + i), a1);
                    }
                }
                i += 0x10;
                a0 += 0x10;
                j += 1;
            } while (j < D_800A33AC);
        }
    }

    D_800A33A0 += a1;
    D_800A33A4 -= a1;
}
