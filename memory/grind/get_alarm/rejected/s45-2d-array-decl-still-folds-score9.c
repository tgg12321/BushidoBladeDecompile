/* REJECTED (s45, synthesis) — 2-D array declaration for D_8009BF68.
 *
 * Probe: `extern s32 D_8009BF68[][1];` with the read spelled `D_8009BF68[0][0]`,
 * on the SOTN-libgpu-faithful floor-9 body.  Hypothesis: a two-level ARRAY_REF
 * forces the base address through a pseudo that combine cannot fold, reproducing
 * target's `lui $v0 / addiu $v0,%lo / lw $a1,0($v0)` materialization.
 *
 * MEASURED: sandbox --disable all -> score 9, target_insns 91, build_insns 90,
 * rules_dropped 0 — identical floor-9 fingerprint.  The extra ARRAY_REF level is
 * folded away before combine, so the single-use address pseudo is still collapsed
 * into `(mem/s (symbol_ref))`.  Corroborates the s45 re-attribution: the fold is
 * gated by MEM_VOLATILE_P, not by the shape of the address expression.
 */
s32 get_alarm(void) {
    s32 temp_v0;
    if (g_gpu_vcount < VSync(-1) || g_gpu_draw_count++ > 0xF0000) {
        *g_gpu_stat_reg;
        printf(&g_str_gpu_timeout, (D_8009BF78 - D_8009BF7C) & 0x3F, *g_gpu_stat_reg, *g_gpu_dma_chcr, *g_gpu_dma_madr);
        printf(&D_80016044, D_8009BF68[0][0], D_8009BF6C, D_8009BF70);
        temp_v0 = SetIntrMask(0);
        D_8009BF7C = 0;
        D_8009BF88 = temp_v0;
        D_8009BF78 = D_8009BF7C;
        *g_gpu_dma_chcr = 0x401;
        *D_8009BF64 |= 0x800;
        *g_gpu_stat_reg = 0x02000000;
        *g_gpu_stat_reg = 0x01000000;
        SetIntrMask(D_8009BF88);
        return -1;
    }
    return 0;
}
