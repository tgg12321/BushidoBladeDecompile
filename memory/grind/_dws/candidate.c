/* CANDIDATE — _dws (src/display.c) — SESSION 1, MATCHED.
 * sandbox --disable all = 0 (143/143 insns), zero regfix/asmfix rules,
 * zero cheat constructs; full-build SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa.
 * Independent (non-circular) proof: tmp/grind/_dws/s1/bytecheck.py resolves this
 * object's R_MIPS_HI16/LO16/26 relocations with the real linked symbol addresses
 * and compares all 143 words against the shipped encodings carried in
 * asm/funcs/_dws.s's per-line comments — 0 mismatched words.
 *
 * Applies verbatim at src/display.c, replacing the `_GpuChunkHdr_CE0C` typedef
 * and `INCLUDE_ASM("asm/funcs", _dws);`.
 */
/* The GPU rectangle Sony's libgpu passes to _clr / _dws / _drs.  Evidence:
 * the target loads/stores halfwords at +0x4 (w) and +0x6 (h) and reads the
 * two packed 32-bit GPU packet words at +0x0 (x,y) and +0x4 (w,h). */
typedef struct {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
    /* 0x4 */ s16 w;
    /* 0x6 */ s16 h;
} GpuRect;

/* PsyQ libgpu sys.c `_dws` - "data write short", the GPU-side body of
 * LoadImage(): clamp the destination rect to the VRAM page, push the
 * CPU->VRAM copy command plus the odd (non-multiple-of-16) leading words
 * through GP0, then hand the 16-word-aligned bulk to DMA channel 2.
 * Reconstructed from the two version-correct matching decomps of this same
 * Sony function: sotn-decomp src/main/psxsdk/libgpu/sys.c:608-655 (PSX,
 * GCC 2.7.2) and psyz decomp/src/libgpu/sys.c:745-785 (PsyQ 4.0).  Both ship
 * the same `var_s4` transfer-direction selector and the same `% 16` / `/ 16`
 * split; the spelling here follows them. */
s32 _dws(GpuRect *rect, s32 *data) {
    s32 to_write;
    s32 size;
    s32 var_s0;
    s32 var_s4;

    var_s4 = 0;
    set_alarm();
    rect->w = rect->w < 0 ? 0 : (rect->w > D_8009BE78 ? D_8009BE78 : rect->w);
    rect->h = rect->h < 0 ? 0 : (rect->h > D_8009BE7A ? D_8009BE7A : rect->h);
    to_write = (rect->w * rect->h + 1) / 2;
    if (to_write <= 0) {
        return -1;
    }
    var_s0 = to_write % 16;
    size = to_write / 16;

    while (!(*g_gpu_stat_reg & 0x04000000)) {
        if (get_alarm() != 0) {
            return -1;
        }
    }

    *g_gpu_stat_reg = 0x04000000;
    *g_gpu_data_reg = 0x01000000;
    *g_gpu_data_reg = var_s4 ? 0xB0000000 : 0xA0000000;
    *g_gpu_data_reg = *(s32 *)&rect->x;
    *g_gpu_data_reg = *(s32 *)&rect->w;

    while (--var_s0 != -1) {
        *g_gpu_data_reg = *data++;
    }

    if (size) {
        *g_gpu_stat_reg = 0x04000002;
        *g_gpu_dma_madr = (u32)data;
        *g_gpu_dma_bcr = size << 16 | 0x10;
        *g_gpu_dma_chcr = 0x01000201;
    }
    return 0;
}
