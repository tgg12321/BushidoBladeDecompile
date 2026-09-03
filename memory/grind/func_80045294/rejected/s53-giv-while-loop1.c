/* s53 REJECTED (score 1/83, residual identical to candidate.c). Loop 1 written as a top-tested while with the offset left to loop.c strength reduction, on the a0-as-pointer chassis. duplicate_loop_exit_test (jump.c:626) creates the entry guard in the FIRST jump pass, i.e. before cse, so the block-0 shift is still a pre-cse insn and is still canonicalised to i. The guarded do-while (G2) and for (G3) spellings measure the same. */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 v1;
    s32 i = a0;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + (a0 << 4));
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;

    while (i < count) {
        sum += *(s32 *)((u8 *)&D_800EED18 + (i << 4));
        i += 1;
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
