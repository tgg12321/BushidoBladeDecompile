/*
 * s7 — REJECTED, and the whole loop-NOTE family dies with it. score 23 / 47.
 * do { ...flag block... } while (0) around each block, all accesses through one
 * pointer local.  cse.c:8054 really does end the basic block at
 * NOTE_INSN_LOOP_END, and the s7 RTL dump proves it works in cse1:
 * s7/rtl/v3_dw_one_ptr/fn/cse.fn keeps `(set (reg 85) (mem:QI (reg 73)))` for
 * blocks 2 and 3 — the reloads SURVIVE the first cse pass.
 * They are then destroyed by cse2: cse.c:8051-8056 says "If we are running
 * after loop.c has finished, we can IGNORE the NOTE_INSN_LOOP_END", so the
 * second cse pass (after_loop = 1) merges the whole flag section into one basic
 * block again and forwards every store into the following load
 * (cse2.fn has no surviving (mem:QI (reg 73)) read after the &0xF8 one).
 * Consequence: NO loop-note-based boundary — do-while(0), and any construct
 * whose only cse effect is a loop note — can ever suppress the forward here.
 * Only a CODE_LABEL boundary survives cse2 (cse.c:8039, unconditional).
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    s32 val;
    u8 val2;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    do {
        c = p[8] & 1;
        val = *ptr;
        val2 = val | 1;
        if (!c) {
            val2 = val;
        }
        *ptr = val2;
    } while (0);

    do {
        c = p[8] & 2;
        val = *ptr;
        val2 = val | 2;
        if (!c) {
            val2 = val;
        }
        *ptr = val2;
    } while (0);

    do {
        c = p[8] & 4;
        val = *ptr;
        val2 = val | 4;
        if (!c) {
            val2 = val;
        }
        *ptr = val2;
    } while (0);

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
