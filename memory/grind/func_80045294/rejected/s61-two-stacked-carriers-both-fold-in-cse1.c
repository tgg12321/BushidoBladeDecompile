/* s61 REJECTED (dumps): two consecutive `if (i != a0) { i = a0; }` carriers; cse_main (cse.c:8360-8373) reprocesses the block from its start when a jump is altered, so once carrier 0 folds, carrier 1 is reached with the copy in the table and folds in cse1 too; cse2 shift = 75. Not sandboxed. */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 i = a0;
    s32 v1;
    s32 s4;
    s32 count;
    s32 s5;

    /* MEASUREMENT-ONLY carriers (unsanctioned family): two always-false conditionals around re-stores of i.
       carrier 0 folds in cse1 (copy in its EBB); its label L0 is a cse1 EBB boundary that shields carrier 1
       from cse1; carrier 1 folds in cse2 once L0 is gone; both are deleted before flow/sched. */
    if (i != a0) { i = a0; }
    if (i != a0) { i = a0; }
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
