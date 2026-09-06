/* s61 REJECTED score 2 / 83. do{}while(0) LOOP_END splits cse1 so the carrier `if (i != a0) { i = a0; }` survives cse1, is folded by cse2 (cse_jumps_altered -> jump_optimize deletes branch/body/label before flow); shift reads reg 72 through greg (sll $3,$18,4 = target operand, copy-first RTL). Loses because sched.c:2081-2103 makes the first insn after a LOOP_BEG/LOOP_END note (now the sll) depend on every earlier register use/set, so the sll is emitted after sw $31/$21/$20. Unsanctioned carrier family; measurement only. */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 i = a0;
    s32 v1;
    s32 s4;
    s32 count;
    s32 s5;

    /* FAKE: do-while(0) wrap, mechanism: NOTE_INSN_LOOP_END ends cse1's extended basic block (cse.c cse_end_of_basic_block, !after_loop), lever-exhaustion: hypotheses.md s52-s60 */
    do { } while (0);
    /* MEASUREMENT-ONLY carrier (unsanctioned family): always-false conditional around a re-store of i */
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
