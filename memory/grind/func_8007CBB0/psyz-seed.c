/* ===========================================================================
 * DRAFT TRANSPLANT — func_8007CBB0  ( = PsyQ libgpu sys.c `_clr` )
 * 0x8007CBB0 · src/display.c:682 (currently INCLUDE_ASM) · dist 151 · 0 rules
 *
 * (a) psyz source: tmp/psyq_prov/psyz/decomp/src/libgpu/sys.c:706-741
 * (b) Read _libgpu_sys_shared_decls.h in this directory FIRST — it carries the
 *     full symbol map and the GpuRect / GpuDrEnv declarations this file needs,
 *     including the split-scalars-hide-aggregate write-up for D_800F1858.
 *
 * MAPPING DECISIONS SPECIFIC TO THIS FUNCTION
 *   psyz                                  BB2
 *   ------------------------------------  --------------------------------
 *   CLAMP(rect->w, 0, info.w - 1)         inlined ternary on D_8009BE78 - 1
 *                                         (BB2 has no CLAMP macro; the
 *                                          already-matched func_8007C8xx pair
 *                                          at display.c:558/576 spells the
 *                                          identical clamp as a bare nested
 *                                          ternary — MATCHED THAT STYLE)
 *   *GPU_STATUS                           *g_gpu_stat_reg
 *   _param(n)                             gpu_GetInfo(n)
 *   _cwc(&D_800E8640)                     gpu_StartDmaList((u32)&D_800F1858)
 *   CMD_MONOCHROME_RECTANGLE(color)       (color & 0xFFFFFF) | 0x60000000
 *   CMD_FILL_RECTANGLE_IN_VRAM(color)     (color & 0xFFFFFF) | 0x02000000
 *   (BB2's include/psx.h has GP1_* / DMA_* names but NO GP0 command names, so
 *    the raw hex is the BB2-idiomatic spelling here — display.c already writes
 *    0xE1/0xE2/0xE4 constants literally, e.g. display.c:568.)
 *
 * ASM CROSS-CHECK (asm/funcs/func_8007CBB0.s, 151 insns, 0 back-edges):
 *   - clamp w against D_8009BE78-1, clamp h against D_8009BE7A-1  .. lines 6-38
 *   - `lhu 0x0` & 0x3F  bnez -> unaligned;  `lhu 0x4` & 0x3F  beqz -> aligned
 *   - unaligned arm stores 12 words; `&D_800F187C` materialised into $a2, and
 *     $a2 is reused for the code[8] store — the aggregate signature
 *   - three `jal gpu_GetInfo` with $a0 = 3, 4, 5, OR'd with 0xE3/0xE4/0xE5
 *   - both arms fall into `jal gpu_StartDmaList` with $a0 = &D_800F1858
 *   - returns 0 (`addu $v0, $zero, $zero`)
 *   All 8 GP0 constants in the sweep's structural table are present:
 *   0xE1/0xE3/0xE4/0xE5/0xE6000000, 0xE4FFFFFF, 0x7FF, 0xFFFFFF.
 *
 * (c) OPEN QUESTIONS FOR THE MEASURING SESSION
 *   1. `color` MUST be u32 — the shift is `srl $v0,$t1,31` (logical). A signed
 *      s32 gives `sra` and a 2-instruction miss. Same in both arms.
 *   2. Store ORDER inside each arm is the load-bearing part; GCC 2.7.2 will
 *      schedule across the stores, so if the residual is a permutation of
 *      correct stores that is a scheduler problem, not a source problem.
 *   3. The tag expression `((u32)&D_800F1858.code[8] & 0xFFFFFF) | 0x08000000`
 *      is what forces the address into a register.  Do not "simplify" it to a
 *      constant — that kills the $a2 reuse.
 *   4. This replaces the whole INCLUDE_ASM line at src/display.c:682 and the
 *      _GpuChunkHdr_CE0C typedef at 683-687 (shared with func_8007CE0C).
 * ======================================================================== */

s32 func_8007CBB0(GpuRect *rect, u32 color) {
    u32 ptr;

    rect->w = rect->w < 0 ? 0
                          : (rect->w > D_8009BE78 - 1 ? D_8009BE78 - 1 : rect->w);
    rect->h = rect->h < 0 ? 0
                          : (rect->h > D_8009BE7A - 1 ? D_8009BE7A - 1 : rect->h);

    if (rect->x & 0x3F || rect->w & 0x3F) {
        /* unaligned clear — split into two packets */
        ptr = (u32)&D_800F1858.code[8];
        D_800F1858.tag = (ptr & 0xFFFFFF) | 0x08000000;
        D_800F1858.code[0] = 0xE3000000;            /* draw area top-left    */
        D_800F1858.code[1] = 0xE4FFFFFF;            /* draw area bottom-right*/
        D_800F1858.code[2] = 0xE5000000;            /* draw offset           */
        D_800F1858.code[3] = 0xE6000000;            /* mask bit setting      */
        D_800F1858.code[4] =
            0xE1000000 | *g_gpu_stat_reg & 0x7FF | (color >> 0x1F) << 10;
        D_800F1858.code[5] = (color & 0xFFFFFF) | 0x60000000;
        D_800F1858.code[6] = *(s32 *)&rect->x;
        D_800F1858.code[7] = *(s32 *)&rect->w;
        D_800F1858.code[8] = 0xFFFFFF | 0x03000000;
        D_800F1858.code[9] = gpu_GetInfo(3) | 0xE3000000;
        D_800F1858.code[10] = gpu_GetInfo(4) | 0xE4000000;
        D_800F1858.code[11] = gpu_GetInfo(5) | 0xE5000000;
    } else {
        /* aligned clear */
        D_800F1858.tag = 0xFFFFFF | 0x05000000;
        D_800F1858.code[0] = 0xE6000000;
        D_800F1858.code[1] =
            0xE1000000 | *g_gpu_stat_reg & 0x7FF | (color >> 0x1F) << 10;
        D_800F1858.code[2] = (color & 0xFFFFFF) | 0x02000000;
        D_800F1858.code[3] = *(s32 *)&rect->x;
        D_800F1858.code[4] = *(s32 *)&rect->w;
    }
    gpu_StartDmaList((u32)&D_800F1858);
    return 0;
}
