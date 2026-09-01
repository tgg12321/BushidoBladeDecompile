/*
 * REJECTED — func_8003C714 — s3 (2026-09-01, structural)
 * SLUG: byte-neutral-masks-fold-before-loop
 *
 * WHY IT IS DEAD: this is the "zero-cost insn_count inflation" probe. K8 (s2)
 * measured that a MATERIALIZING respelling costs ~6.5 score points per +1 RTL
 * insn, so the open sub-axis was whether an ordinary-C construct could add RTL
 * insns that loop.c COUNTS but that a later pass folds away at zero byte cost
 * (loop.c:1631 needs insn_count >= 120 on the shipped no-call chassis).
 * This form adds an explicit `& 0xFF` to each of the four stored values — the
 * stores are to u8 so the mask is semantically redundant and byte-neutral.
 *
 * MEASURED: sandbox score 15 (105 insns) — byte-neutral as designed — and the
 * .loop dump still prints `Loop from 25 to 146: 56 real insns.` with all three
 * movables moved. insn_count is UNCHANGED: +0. The masks are folded away at
 * expand/cse1, i.e. BEFORE loop.c ever counts the loop, so they contribute
 * nothing to insn_count. The folding channel that would make inflation free
 * runs upstream of the pass that has to see the insns.
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    u8 *src;
    u8 *dst;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    do {
        src = (u8 *)&D_80106A58 + i * 8;
        dst = (u8 *)s0 + i * 4;
        dst[0x21] = (*(s32 *)(src + 4) / 1800) & 0xFF;
        dst[0x22] = ((*(s32 *)(src + 4) / 30) % 60) & 0xFF;
        dst[0x23] = (((*(s32 *)(src + 4) % 30) * 100) / 30) & 0xFF;
        dst[0x24] = *src & 0xFF;
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
