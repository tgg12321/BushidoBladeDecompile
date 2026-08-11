/* BEST ADAPTED BODY - src/display.c - func_8007D048 == LIBGPU/SYS `_drs` (StoreImage)
 * Reference: sotn-decomp src/main/psxsdk/libgpu/sys.c:660-712, cross-checked against
 * BB2's OWN already-decompiled sibling func_8007CE0C (`_dws`, LoadImage) for the BB2
 * symbol vocabulary (g_gpu_stat_reg/data_reg/dma_*, D_8009BE78/7A clamps,
 * func_8007DC68 = set_alarm, func_8007DC9C = get_alarm).
 * Deltas vs _dws: writes 0xC0000000 (VRAM->CPU) unconditionally, adds the second
 * "wait for 0x08000000 ready-to-send" spin, reads the FIFO into arg1, and uses DMA
 * direction 0x04000003 / chcr 0x01000200.
 * NB the sibling's register pin + `__asm__` barrier were deliberately NOT copied.
 *
 * MEASURED: 44   (baseline 161 = INCLUDE_ASM).  build 160 / target 161.
 * RESIDUAL: the clamp head - target keeps a copy (`addu $a0,$v0,$zero`) that our build
 * folds into the slt, plus callee-save renames. Target frame is 0x48, ours 0x38
 * (phantom slots). This head residual is SHARED with the sibling func_8007CE0C, which
 * BB2 already carries at score 48.
 *
 * Replaces:  INCLUDE_ASM("asm/funcs", func_8007D048);
 * Reuses the _GpuChunkHdr_CE0C typedef already declared above func_8007CE0C.
 */

s32 func_8007D048(_GpuChunkHdr_CE0C *arg0, s32 *arg1) {
    s16 coord;
    s32 half_size;
    s32 big_size;
    s32 remainder;
    s32 v1_tmp;
    u16 a0_tmp;
    s32 v0_ext;

    func_8007DC68();

    coord = (v1_tmp = arg0->x);
    if (coord < 0) goto x_neg;
    if (D_8009BE78 < coord) {
        v1_tmp = D_8009BE78;
    }
    goto x_done;
x_neg:
    v1_tmp = 0;
x_done:
    arg0->x = (s16)v1_tmp;

    coord = arg0->y;
    if (coord < 0) goto y_neg;
    a0_tmp = coord;
    v0_ext = a0_tmp << 16;
    if (D_8009BE7A < coord) {
        a0_tmp = D_8009BE7A;
        goto y_block_8;
    }
    goto y_done;
y_neg:
    a0_tmp = 0;
y_block_8:
    v0_ext = a0_tmp << 16;
y_done:
    arg0->y = (s16)a0_tmp;

    {
        s32 y_ext = v0_ext >> 16;
        s32 prod = (s32)arg0->x * y_ext + 1;
        s32 rounded = prod + ((u32)prod >> 31);
        half_size = rounded >> 1;
        big_size = rounded >> 5;
    }

    if (half_size <= 0) {
        return -1;
    }

    remainder = half_size - big_size * 16;

    if (!((*g_gpu_stat_reg) & 0x04000000)) {
        do {
            if (func_8007DC9C() != 0) {
                return -1;
            }
        } while (!((*g_gpu_stat_reg) & 0x04000000));
    }

    *g_gpu_stat_reg = 0x04000000;
    *g_gpu_data_reg = 0x01000000;
    *g_gpu_data_reg = 0xC0000000;
    *g_gpu_data_reg = arg0->unk0;
    *g_gpu_data_reg = *(s32 *)&arg0->x;

    if (!((*g_gpu_stat_reg) & 0x08000000)) {
        do {
            if (func_8007DC9C() != 0) {
                return -1;
            }
        } while (!((*g_gpu_stat_reg) & 0x08000000));
    }

    while (--remainder != -1) {
        *arg1++ = *g_gpu_data_reg;
    }

    if (big_size != 0) {
        *g_gpu_stat_reg = 0x04000003;
        *g_gpu_dma_madr = (u32)arg1;
        *g_gpu_dma_bcr = (big_size << 16) | 0x10;
        *g_gpu_dma_chcr = 0x01000200;
    }

    return 0;
}
