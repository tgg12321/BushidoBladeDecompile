/* s53 REJECTED (score 31, build_insns 80). Loop-1 counter distinct from loop-2's counter with 'sum' reused as loop 2's byte offset: the loop-1 counter then crosses no call, local-alloc leaves it in the incoming $a0, block 0's move $s0,$s2 disappears and the $s5 save/restore pair drops out (the s48/s51 allocation collapse). Confirms the block-0 copy's dest must itself cross DrawSync/func_800520B8. */
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
        s32 j;

        gpu_DrawSync(0);
        func_800520B8(s4, s5, sum);

        j = a0;
        if (j < D_800A33AC) {
            v1 = j << 4;
            a0 = (s32)((u8 *)&D_800EED14 + v1);
            sum = v1;
            do {
                *(s32 *)a0 += a1;
                {
                    void (*fn)(s16, s32) = (void (*)(s16, s32)) *(s32 *)((u8 *)&D_800EED1C + sum);
                    if (fn != 0) {
                        fn(*(s16 *)((u8 *)&D_800EED10 + sum), a1);
                    }
                }
                sum += 0x10;
                a0 += 0x10;
                j += 1;
            } while (j < D_800A33AC);
        }
    }

    D_800A33A0 += a1;
    D_800A33A4 -= a1;
}
