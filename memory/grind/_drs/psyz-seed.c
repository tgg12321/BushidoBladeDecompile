/* ===========================================================================
 * DRAFT TRANSPLANT — func_8007D048  ( = PsyQ libgpu sys.c `_drs`, StoreImage )
 * 0x8007D048 · src/display.c:772 (currently INCLUDE_ASM) · dist 161 · 0 rules
 *
 * (a) psyz source: tmp/psyq_prov/psyz/decomp/src/libgpu/sys.c:787-833
 * (b) Read _libgpu_sys_shared_decls.h FIRST for the symbol map + GpuRect.
 *
 * MAPPING DECISIONS — identical to func_8007CE0C (_dws); see that file's table.
 * The only differences from _dws are structural, and all three are confirmed
 * in asm/funcs/func_8007D048.s (161 insns, 3 back-edges = psyz's 3 loops):
 *   1. no `var_s4` flag — the direction word is always 0xC0000000
 *      (asm 8007D1A0: `lui $v0,(0xC0000000>>16)`), so this function has NONE
 *      of _dws's constant-fold risk.  It is the safer of the pair to measure
 *      first if you want an early signal on the shared clamp/divide shape.
 *   2. a SECOND wait loop after the packet, on status bit 0x08000000
 *      (asm 8007D1E0: `lui $v1,(0x8000000>>16)`) — VRAM-ready-to-send.
 *   3. the tail DMA words are 0x04000003 / 0x01000200 (vs _dws's
 *      0x04000002 / 0x01000201) — the sweep's "shared 0x01000200 / 0x04000003
 *      DMA words" evidence.
 *
 * ASM CROSS-CHECK of the shared prologue (lines 12-58 of the .s):
 *   jal func_8007DC68 ; lh 0x4($s1) ; bltz -> 0 ; else slt vs D_8009BE78,
 *   beqz keeps original ; sh 0x4 ; same again for 0x6 vs D_8009BE7A ;
 *   sll/sra 16 sign-extend of h ; mult ; addiu 1 ; srl 31 ; addu ; sra 1 ;
 *   sra 5 ; bgtz -> continue else return -1.  This is byte-for-byte the same
 *   opening as func_8007CE0C, which is the strongest argument that the two
 *   should be measured as a unit: whatever spelling of the clamp+divide lands
 *   one lands the other.
 *
 * (c) OPEN QUESTIONS FOR THE MEASURING SESSION
 *   1. `var_s0 = to_read % 16; size = to_read / 16;` — the asm computes
 *      `sra $s0,$v1,5` (i.e. /32 of the pre-halved value) and then
 *      `subu $s0,$a0,$v0` with `sll $v0,$v1,4`.  That is GCC's own expansion
 *      of `%16` given `size` already in a register; write the divisions, do
 *      not hand-expand.
 *   2. The read loop is `*data++ = *GPU_DATA;` — data is `s32 *`; if the
 *      residual shows an lw/sw pairing mismatch check the volatile qualifier
 *      on g_gpu_data_reg (it is `volatile u32 *` in display.c:19).
 *   3. This replaces the INCLUDE_ASM line at src/display.c:772 only.
 * ======================================================================== */

s32 func_8007D048(GpuRect *rect, s32 *data) {
    s32 to_read;
    s32 size;
    s32 var_s0;

    func_8007DC68();
    rect->w = rect->w < 0 ? 0 : (rect->w > D_8009BE78 ? D_8009BE78 : rect->w);
    rect->h = rect->h < 0 ? 0 : (rect->h > D_8009BE7A ? D_8009BE7A : rect->h);
    to_read = (rect->w * rect->h + 1) / 2;
    if (to_read <= 0) {
        return -1;
    }
    var_s0 = to_read % 16;
    size = to_read / 16;

    while (!(*g_gpu_stat_reg & 0x04000000)) {
        if (func_8007DC9C() != 0) {
            return -1;
        }
    }

    *g_gpu_stat_reg = 0x04000000;
    *g_gpu_data_reg = 0x01000000;
    *g_gpu_data_reg = 0xC0000000;
    *g_gpu_data_reg = *(s32 *)&rect->x;
    *g_gpu_data_reg = *(s32 *)&rect->w;

    while (!(*g_gpu_stat_reg & 0x08000000)) {
        if (func_8007DC9C() != 0) {
            return -1;
        }
    }

    while (var_s0--) {
        *data++ = *g_gpu_data_reg;
    }

    if (size != 0) {
        *g_gpu_stat_reg = 0x04000003;
        *g_gpu_dma_madr = (u32)data;
        *g_gpu_dma_bcr = (size << 0x10) | 0x10;
        *g_gpu_dma_chcr = 0x01000200;
    }
    return 0;
}
