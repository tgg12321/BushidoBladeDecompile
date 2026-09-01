/*
 * REJECTED — inline index-derived address expressions (drop the `src` local and
 * respell each read as `*(s32 *)((u8 *)&D_80106A58 + i * 8 + 4)`).
 * s2b (2026-09-01, structural). MEASURED: sandbox == 28 (108 insns) vs the
 * candidate's 15 (105 insns). Loop RTL insn_count 56 -> 58.
 *
 * PURPOSE OF THE PROBE: quantify the ONLY remaining lever on loop.c's
 * desirability test `threshold * savings * m->lifetime >= insn_count`
 * (loop.c:1631) for the 0x91A2B3C5 movable — namely raising insn_count. On the
 * shipped chassis threshold is 122 (loop.c:532, n_non_fixed_regs == 60) and 119
 * after the first hoist's `threshold -= 3`, so the movable only stays in-loop
 * once insn_count >= 120.
 * RESULT: +2 RTL insns cost +13 score / +3 machine insns — i.e. roughly 6.5
 * score points per RTL insn of loop inflation. Closing the remaining 62 RTL
 * insns would cost hundreds of score points in a function whose target body is
 * 104 instructions total. The insn_count axis is quantitatively dead, not just
 * argued dead. Corollary: hypotheses.md K8.
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    u8 *dst;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    do {
        dst = (u8 *)s0 + i * 4;
        dst[0x21] = *(s32 *)((u8 *)&D_80106A58 + i * 8 + 4) / 1800;
        dst[0x22] = (*(s32 *)((u8 *)&D_80106A58 + i * 8 + 4) / 30) % 60;
        dst[0x23] = ((*(s32 *)((u8 *)&D_80106A58 + i * 8 + 4) % 30) * 100) / 30;
        dst[0x24] = *((u8 *)&D_80106A58 + i * 8);
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
