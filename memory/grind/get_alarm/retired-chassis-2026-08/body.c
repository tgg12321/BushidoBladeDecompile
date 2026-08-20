s32 get_alarm(void) {
    volatile s32 *new_var2;
    s32 temp_v0;
    s32 temp_v1;
    s32 new_var;
    new_var2 = &D_8009BF7C;
    if ((g_gpu_vcount < VSync(-1)) || ((temp_v1 = g_gpu_draw_count, g_gpu_draw_count = temp_v1 + 1, (temp_v1 > 0xF0000) != 0))) {
        new_var = *g_gpu_stat_reg;
        printf(&g_str_gpu_timeout, (D_8009BF78 - D_8009BF7C) & 0x3F, *g_gpu_stat_reg, *g_gpu_dma_chcr, *g_gpu_dma_madr);
        (void)new_var;
        printf(&D_80016044, D_8009BF68[0], D_8009BF6C, D_8009BF70);
        temp_v0 = SetIntrMask(0);
        D_8009BF7C = 0;
        D_8009BF88 = temp_v0;
        D_8009BF78 = *new_var2;
        *g_gpu_dma_chcr = 0x401;
        *D_8009BF64 |= 0x800;
        *g_gpu_stat_reg = 0x02000000;
        *g_gpu_stat_reg = 0x01000000;
        SetIntrMask(D_8009BF88);
        return -1;
    }
    return 0;
}
