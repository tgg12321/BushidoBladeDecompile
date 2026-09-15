/* REJECTED (s1, 2026-09-15): sandbox 23/152 -- thirteen per-word u32 scalars for the
 * DR_ENV buffer.  The target takes &code[8] (0x800F187C) into $a2 and stores 0x03FFFFFF
 * through it; independent scalars cannot share that base register (extra lui + drift).
 * The aggregate `GpuDrEnv D_800F1858` form scores 0/151 (candidate.c). */
extern u32 D_800F1858;
extern u32 D_800F185C;
extern u32 D_800F1860;
extern u32 D_800F1864;
extern u32 D_800F1868;
extern u32 D_800F186C;
extern u32 D_800F1870;
extern u32 D_800F1874;
extern u32 D_800F1878;
extern u32 D_800F187C;
extern u32 D_800F1880;
extern u32 D_800F1884;
extern u32 D_800F1888;

/* 0x8009BF48 (= g_gpu_stat_reg, display.c:18) spelled by its splat name here,
 * as _addque2/_reset (display.c:773/842) already do for the same word. */
extern volatile s32 *D_8009BF48;
void _cwc(u32 a0);
u32 _param(u32 a0);

/* PsyQ libgpu sys.c `_clr` - the GPU-side body of ClearImage(): clamp the
 * rect to the VRAM page, build either a 12-word unaligned (mono rectangle)
 * or 5-word aligned (VRAM fill) packet in the DR_ENV buffer, and DMA it.
 * Spelling follows psyz decomp/src/libgpu/sys.c:706-741 (PsyQ 4.0). */
s32 _clr(GpuRect *rect, u32 color) {
    u32 ptr;

    rect->w = rect->w < 0 ? 0 : (rect->w > D_8009BE78 - 1 ? D_8009BE78 - 1 : rect->w);
    rect->h = rect->h < 0 ? 0 : (rect->h > D_8009BE7A - 1 ? D_8009BE7A - 1 : rect->h);
    if (rect->x & 0x3F || rect->w & 0x3F) {
        /* unaligned clear: split in two packets */
        ptr = (u32)&D_800F187C;
        D_800F1858 = (ptr & 0xFFFFFF) | 0x08000000;
        D_800F185C = 0xE3000000;
        D_800F1860 = 0xE4FFFFFF;
        D_800F1864 = 0xE5000000;
        D_800F1868 = 0xE6000000;
        D_800F186C = 0xE1000000 | *D_8009BF48 & 0x7FF | (color >> 0x1F) << 10;
        D_800F1870 = (color & 0xFFFFFF) | 0x60000000;
        D_800F1874 = *(s32 *)&rect->x;
        D_800F1878 = *(s32 *)&rect->w;
        D_800F187C = 0xFFFFFF | 0x03000000;
        D_800F1880 = _param(3) | 0xE3000000;
        D_800F1884 = _param(4) | 0xE4000000;
        D_800F1888 = _param(5) | 0xE5000000;
    } else {
        /* aligned clear */
        D_800F1858 = 0xFFFFFF | 0x05000000;
        D_800F185C = 0xE6000000;
        D_800F1860 = 0xE1000000 | *D_8009BF48 & 0x7FF | (color >> 0x1F) << 10;
        D_800F1864 = (color & 0xFFFFFF) | 0x02000000;
        D_800F1868 = *(s32 *)&rect->x;
        D_800F186C = *(s32 *)&rect->w;
    }
    _cwc((u32)&D_800F1858);
    return 0;
}
