/*
 * s7 — REJECTED. score 31 / 36 insns (symbol-spelled twin v10: 31 / 37).
 * The flag section written as a loop over the bit index, i.e. the LAST untried
 * SHAPE on this function (s5 frontier F2 / s6 frontier F1).
 * GCC 2.7.2 at -O2 without -funroll-loops neither unrolls nor peels it: the
 * objdump contains one real loop body (36 insns against target's 49, three
 * flag stores collapsed into one) so the shape is refuted directly by the
 * bytes, not by the score.  The loop DOES do what the frontier predicted for
 * cse — the back-edge CODE_LABEL ends cse's basic block and the loop-carried
 * read of the flag byte survives as a real lbu in BOTH cse.fn and cse2.fn —
 * but an un-unrolled loop can never match an unrolled 49-insn target.
 * The reusable half of this result is banked as p1_loop1_per_block.
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    s32 val;
    u8 val2;
    s32 c;
    s32 k;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    for (k = 1; k <= 4; k <<= 1) {
        c = p[8] & k;
        val = *ptr;
        val2 = val | k;
        if (!c) {
            val2 = val;
        }
        *ptr = val2;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
