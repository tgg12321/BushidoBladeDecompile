/* KILLED s2: hoisting `s5 = s4 + a1;` INTO if(sum!=0) block loses block0's
 * addu $21 delay-slot fill; build shrinks to 79 insns (target 83), score
 * rose 2 -> 13. s5 must be computed BEFORE the guard branch so reorg.c can
 * place addu $21,$20,$19 in the beqz delay slot. */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 v1 = a0 << 4;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 i = a0;
    s32 count = D_800A33AC;

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
        s32 s5 = s4 + a1;

        gpu_DrawSync(0);
        func_800520B8(s4, s5, sum);
        /* ... same as candidate ... */
    }
}
