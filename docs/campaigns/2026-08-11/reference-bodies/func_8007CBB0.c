/* BEST ADAPTED BODY - src/display.c - func_8007CBB0 == LIBGPU/SYS `_clr` (ClearImage)
 * Reference: sotn-decomp src/main/psxsdk/libgpu/sys.c:567-605.
 * BB2 is an older library revision: the packet buffer is the fixed global
 * D_800F1858[13] (SOTN uses D_80037E20[12]); the otag word is 0x08000000|&buf[9]
 * (SOTN 0x07000000|&buf[8]); the clamps use the globals D_8009BE78/D_8009BE7A minus 1
 * (SOTN uses 0x400/0x200 literals); _param(n) -> gpu_GetInfo(n); _cwc() ->
 * gpu_StartDmaList(). BB2 additionally emits a 0xE1000000 texpage word that SOTN's
 * revision does not have.
 *
 * MEASURED: 77   (baseline 151 = INCLUDE_ASM).  build 149 / target 151.
 * RESIDUAL: register allocation across the two packet-fill arms plus two missing
 * instructions in the clamp head (same clamp-copy fold as func_8007D048).
 *
 * Replaces:  INCLUDE_ASM("asm/funcs", func_8007CBB0);
 */

typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} _GpuRect_CBB0;

extern u32 D_800F1858[];
extern u32 gpu_GetInfo(u32);
extern void gpu_StartDmaList(u32);

s32 func_8007CBB0(_GpuRect_CBB0 *arg0, s32 color) {
    s16 t;
    s32 v;

    t = arg0->w;
    if (t < 0) {
        v = 0;
    } else if (D_8009BE78 - 1 < t) {
        v = D_8009BE78 - 1;
    } else {
        v = t;
    }
    arg0->w = (s16)v;

    t = arg0->h;
    if (t < 0) {
        v = 0;
    } else if (D_8009BE7A - 1 < t) {
        v = D_8009BE7A - 1;
    } else {
        v = t;
    }
    arg0->h = (s16)v;

    if ((*(u16 *)&arg0->x & 0x3F) || (*(u16 *)&arg0->w & 0x3F)) {
        D_800F1858[0] = ((u32)&D_800F1858[9] & 0xFFFFFF) | 0x08000000;
        D_800F1858[1] = 0xE3000000;
        D_800F1858[2] = 0xE4FFFFFF;
        D_800F1858[3] = 0xE5000000;
        D_800F1858[4] = 0xE6000000;
        D_800F1858[5] = (*g_gpu_stat_reg & 0x7FF) |
                        ((((u32)color >> 31) << 10) | 0xE1000000);
        D_800F1858[6] = (color & 0xFFFFFF) | 0x60000000;
        D_800F1858[7] = *(u32 *)&arg0->x;
        D_800F1858[8] = *(u32 *)&arg0->w;
        D_800F1858[9] = 0x03FFFFFF;
        D_800F1858[10] = gpu_GetInfo(3) | 0xE3000000;
        D_800F1858[11] = gpu_GetInfo(4) | 0xE4000000;
        D_800F1858[12] = gpu_GetInfo(5) | 0xE5000000;
    } else {
        D_800F1858[0] = 0x05FFFFFF;
        D_800F1858[1] = 0xE6000000;
        D_800F1858[3] = (color & 0xFFFFFF) | 0x02000000;
        D_800F1858[2] = (*g_gpu_stat_reg & 0x7FF) |
                        ((((u32)color >> 31) << 10) | 0xE1000000);
        D_800F1858[4] = *(u32 *)&arg0->x;
        D_800F1858[5] = *(u32 *)&arg0->w;
    }
    gpu_StartDmaList((u32)&D_800F1858[0]);
    return 0;
}
