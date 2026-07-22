
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
extern s32 sys_VSync(s32);
extern volatile u32 *g_gpu_stat_reg;
extern volatile u32 *g_gpu_dma_chcr;
extern u32 *g_gpu_dma_madr;
extern s32 g_gpu_vcount;
extern s32 g_gpu_draw_count;
extern s32 motion_make_table(s32);
extern volatile s32 D_8009BF7C;
extern s32 D_8009BF78;
extern s32 D_8009BF88;
extern s32 g_str_gpu_timeout;
extern s32 D_80016044;
extern volatile int *D_8009BF64;
extern volatile long D_8009BF68[];
extern s32 D_8009BF6C;
extern s32 D_8009BF70;
extern s32 debug_printf();
s32 func_8007DC9C(void)
{
  s32 temp_v0;
  s32 temp_v1;
  s32 new_var;
  if ((g_gpu_vcount < sys_VSync(-1)) || ((temp_v1 = g_gpu_draw_count, g_gpu_draw_count = temp_v1 + 1, (temp_v1 > 0xF0000) != 0)))
  {
    new_var = *g_gpu_stat_reg;
    debug_printf(&g_str_gpu_timeout, (D_8009BF78 - D_8009BF7C) & 0x3F, *g_gpu_stat_reg, *g_gpu_dma_chcr, *g_gpu_dma_madr);
    (void) new_var;
    debug_printf(&D_80016044, D_8009BF68[0], D_8009BF6C, D_8009BF70);
    temp_v0 = motion_make_table(0);
    D_8009BF7C = 0;
    D_8009BF88 = temp_v0;
    D_8009BF78 = D_8009BF7C;
    *g_gpu_dma_chcr = 0x401;
    *D_8009BF64 |= 0x800;
    *g_gpu_stat_reg = 0x02000000;
    *g_gpu_stat_reg = 0x01000000;
    motion_make_table(D_8009BF88);
    return -1;
  }
  return 0;
}
