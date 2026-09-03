/* s53 REJECTED (score 11/83, HEAD 2026-09-03 a0-as-pointer chassis). Reusing loop 1's counter as loop 2's byte offset makes i's last mention (idx += 0x10) precede a0's (a0 += 0x10), so cse.c:855 clause (2) fails and a0 stays qty_first_reg: block 0 emits the TARGET's sw $s0 / move $s0,$s2 / sll $v1,$s2,4 exactly. Dead because the same crown state rewrites the loop-1 entry guard from i to a0: build emits slt $v0,$s2,$a0 where the target has slt $v0,$s0,$a0 (dumps_A/text1a_c.cse.fn insn 31), and loop 2's callee-saves rotate (idx $s0 / counter $s1 vs target $s1 / $s0). Kept as the proof that both crown states are reachable on this chassis. */
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
