/* PsyQ libgpu sys.c `_drs` - "data read short", the GPU-side body of
 * StoreImage(): clamp the source rect to the VRAM page, push the VRAM->CPU
 * copy command through GP0, wait for the GPU to be ready to send, read the
 * odd (non-multiple-of-16) leading words through GP0, then hand the
 * 16-word-aligned bulk to DMA channel 2.  Same shape as _dws above; spelling
 * follows psyz decomp/src/libgpu/sys.c:787-833 (PsyQ 4.0). */
s32 _drs(GpuRect *rect, s32 *data) {
    s32 to_read;
    s32 size;
    s32 var_s0;

    set_alarm();
    rect->w = rect->w < 0 ? 0 : (rect->w > D_8009BE78 ? D_8009BE78 : rect->w);
    rect->h = rect->h < 0 ? 0 : (rect->h > D_8009BE7A ? D_8009BE7A : rect->h);
    to_read = (rect->w * rect->h + 1) / 2;
    if (to_read <= 0) {
        return -1;
    }
    var_s0 = to_read % 16;
    size = to_read / 16;

    while (!(*g_gpu_stat_reg & 0x04000000)) {
        if (get_alarm() != 0) {
            return -1;
        }
    }

    *g_gpu_stat_reg = 0x04000000;
    *g_gpu_data_reg = 0x01000000;
    *g_gpu_data_reg = 0xC0000000;
    *g_gpu_data_reg = *(s32 *)&rect->x;
    *g_gpu_data_reg = *(s32 *)&rect->w;

    while (!(*g_gpu_stat_reg & 0x08000000)) {
        if (get_alarm() != 0) {
            return -1;
        }
    }

    while (--var_s0 != -1) {
        *data++ = *g_gpu_data_reg;
    }

    if (size) {
        *g_gpu_stat_reg = 0x04000003;
        *g_gpu_dma_madr = (u32)data;
        *g_gpu_dma_bcr = size << 16 | 0x10;
        *g_gpu_dma_chcr = 0x01000200;
    }
    return 0;
}
void _ctl(u32 a0) {
    *g_gpu_stat_reg = a0;
    g_gpu_color_table[a0 >> 24] = a0;
