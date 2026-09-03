/* REJECTED s51: score 27, build_insns 84 (target 83).  i-first block-0 chassis
 * plus a trailing dead store `i = a0;` as the function's last statement, aimed
 * at making uid_cuid[regno_last_uid[a0]] >= uid_cuid[regno_last_uid[i]] so that
 * cse.c:842-857 leaves a0 as qty_first_reg and block 0's shift keeps reading
 * $s2.  The store's destination `i` IS read elsewhere, so jump.c:568-584 does
 * NOT delete it (unlike the dead-local spelling in
 * s51-dead-a0-ref-any-placement.c); it survives every pass into the emitted
 * bytes as an extra `move`, putting the build at 84 instructions.  Also note
 * the store makes regno_last_uid[i] == regno_last_uid[a0] (same insn), so the
 * strict `>` in cse.c:855 is what would have to flip -- but the +1 instruction
 * disqualifies the form before that matters.
 * The FAKE annotation on the store is retained verbatim from the probe; the
 * form is REJECTED, not proposed.
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

        gpu_DrawSync(0);
        func_800520B8(s4, s5, sum);

        i = a0;
        if (i < D_800A33AC) {
            v1 = i << 4;
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
                i += 1;
            } while (i < D_800A33AC);
        }
    }

    D_800A33A0 += a1;
    D_800A33A4 -= a1;
    /* FAKE: dead store, mechanism: cse.c make_regs_eqv, lever-exhaustion: ledger */
    i = a0;
}
