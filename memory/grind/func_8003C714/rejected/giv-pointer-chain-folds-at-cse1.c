/*
 * REJECTED - func_8003C714 - s5 (2026-09-01, synthesis modality)
 * SLUG: giv-pointer-chain-folds-at-cse1
 *
 * WHY IT IS DEAD (measured, not argued): chained address temporaries
 * (q0 = dst + 1; q1 = q0 + 1; ...; store through the last one at the residual
 * offset) are folded into the memory reference by cse1, UPSTREAM of
 * loop_optimize. They therefore add ZERO to loop.c's insn_count - the channel
 * this form was built to test (RTL insns that loop.c counts but that a LATER
 * pass deletes for free: strength_reduce's combine_givs, flow's DCE, or
 * combine, all of which run after loop) does not open from this direction.
 *
 * Sweep k = 0, 1, 2, 4, 8, 16 (tmp/grind/func_8003C714/s5/gen_chain.py):
 *   k=0   Loop from 25 to 146: 56 real insns.   asm 107 lines
 *   k=1   Loop from 25 to 149: 56 real insns.   asm 107 lines
 *   k=2   Loop from 25 to 152: 56 real insns.   asm 107 lines
 *   k=4   Loop from 25 to 158: 56 real insns.   asm 107 lines
 *   k=8   Loop from 25 to 170: 56 real insns.   asm 107 lines
 *   k=16  Loop from 25 to 194: 56 real insns.   asm 107 lines
 * insn_count PINNED at 56, emitted function byte-identical at every k.
 *
 * Harness validated in the same session: the identical sweep driver run over
 * s4's gen_k.py reproduces K14 exactly (k=12 -> 82 real insns / all moved;
 * k=13 -> 84 real insns / "not desirable"; k=16 -> 90 real insns, asm 173
 * lines), so the null result above is a real null, not a broken measurement.
 *
 * This is the k=16 form. See hypotheses.md K16.
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    u8 *src;
    u8 *dst;
    u8 *q0, *q1, *q2, *q3, *q4, *q5, *q6, *q7, *q8, *q9, *q10, *q11, *q12, *q13, *q14, *q15;
    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    do {
        src = (u8 *)&D_80106A58 + i * 8;
        dst = (u8 *)s0 + i * 4;
        q0 = dst + 1;
        q1 = q0 + 1;
        q2 = q1 + 1;
        q3 = q2 + 1;
        q4 = q3 + 1;
        q5 = q4 + 1;
        q6 = q5 + 1;
        q7 = q6 + 1;
        q8 = q7 + 1;
        q9 = q8 + 1;
        q10 = q9 + 1;
        q11 = q10 + 1;
        q12 = q11 + 1;
        q13 = q12 + 1;
        q14 = q13 + 1;
        q15 = q14 + 1;
        q15[0x11] = *(s32 *)(src + 4) / 1800;
        dst[0x22] = (*(s32 *)(src + 4) / 30) % 60;
        dst[0x23] = ((*(s32 *)(src + 4) % 30) * 100) / 30;
        dst[0x24] = *src;
        i += 1;
    } while (i < 3);
    func_8001CD68(buf);
    *((u8 *)s0 + 0x2D) = *(u16 *)buf;
    *((u8 *)s0 + 0x2E) = buf[2];
    *((u8 *)s0 + 0x2F) = buf[3];
    *((u8 *)s0 + 0x30) = *(u16 *)&D_80101ED2;
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
