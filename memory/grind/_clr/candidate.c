/* _clr candidate (s1, 2026-09-15): sandbox 0/151 with this body in src/display.c PLUS
 * the GpuDrEnv typedef+extern in include/gpu.h and the 12 alias-row suffixes in
 * undefined_syms_auto.txt -- the exact tree is memory/grind/_clr/candidate_merge.patch
 * (apply to a clean HEAD with git apply). TU-local GpuDrEnv also scores 0 but fails
 * aggregate-merge prong (d). Spelling *g_gpu_stat_reg instead of *D_8009BF48 is the
 * same address (0x8009BF48) but scores a FALSE 4 (scorer symtab blind spot). */
/* The GPU rectangle Sony's libgpu passes to _clr / _dws / _drs.  Evidence:
 * the target loads/stores halfwords at +0x4 (w) and +0x6 (h) and reads the
 * two packed 32-bit GPU packet words at +0x0 (x,y) and +0x4 (w,h). */
typedef struct {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
    /* 0x4 */ s16 w;
    /* 0x6 */ s16 h;
} GpuRect;

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
        ptr = (u32)&D_800F1858.code[8];
        D_800F1858.tag = (ptr & 0xFFFFFF) | 0x08000000;
        D_800F1858.code[0] = 0xE3000000;
        D_800F1858.code[1] = 0xE4FFFFFF;
        D_800F1858.code[2] = 0xE5000000;
        D_800F1858.code[3] = 0xE6000000;
        D_800F1858.code[4] = 0xE1000000 | *D_8009BF48 & 0x7FF | (color >> 0x1F) << 10;
        D_800F1858.code[5] = (color & 0xFFFFFF) | 0x60000000;
        D_800F1858.code[6] = *(s32 *)&rect->x;
        D_800F1858.code[7] = *(s32 *)&rect->w;
        D_800F1858.code[8] = 0xFFFFFF | 0x03000000;
        D_800F1858.code[9] = _param(3) | 0xE3000000;
        D_800F1858.code[10] = _param(4) | 0xE4000000;
        D_800F1858.code[11] = _param(5) | 0xE5000000;
    } else {
        /* aligned clear */
        D_800F1858.tag = 0xFFFFFF | 0x05000000;
        D_800F1858.code[0] = 0xE6000000;
        D_800F1858.code[1] = 0xE1000000 | *D_8009BF48 & 0x7FF | (color >> 0x1F) << 10;
        D_800F1858.code[2] = (color & 0xFFFFFF) | 0x02000000;
        D_800F1858.code[3] = *(s32 *)&rect->x;
        D_800F1858.code[4] = *(s32 *)&rect->w;
    }
    _cwc((u32)&D_800F1858);
    return 0;
}
