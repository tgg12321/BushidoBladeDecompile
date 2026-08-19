/* ===========================================================================
 * SHARED PREAMBLE for the libgpu sys.c unit transplant
 * (func_8007CBB0 / func_8007CE0C / func_8007D048 — all three live in
 *  src/display.c).  This is NOT a file to add to include/; paste the pieces
 *  that are missing into src/display.c near the existing extern block at
 *  src/display.c:18-28.  Everything marked ALREADY PRESENT needs no edit.
 *
 * psyz source for the whole unit:
 *   tmp/psyq_prov/psyz/decomp/src/libgpu/sys.c  (PsyQ 4.0, MIT)
 *
 * SYMBOL MAP — psyz  ->  BB2  (every row verified against asm/funcs/*.s and
 * symbol_addrs.txt; nothing here is inferred from the psyz names alone)
 *
 *   psyz                     BB2                       addr         status
 *   -----------------------  ------------------------  -----------  ---------------
 *   GPU_STATUS (ptr)         g_gpu_stat_reg            0x8009BF48   ALREADY PRESENT
 *   GPU_DATA   (ptr)         g_gpu_data_reg            0x8009BF44   ALREADY PRESENT
 *   DMA2_MADR  (ptr)         g_gpu_dma_madr            0x8009BF4C   ALREADY PRESENT
 *   DMA2_BCR   (ptr)         g_gpu_dma_bcr             0x8009BF50   ALREADY PRESENT
 *   DMA2_CHCR  (ptr)         g_gpu_dma_chcr            0x8009BF54   ALREADY PRESENT
 *   info.version             D_8009BE74  (u8)          0x8009BE74   ALREADY PRESENT
 *   info.reverse             D_8009BE77  (u8)          0x8009BE77   ALREADY PRESENT
 *   info.w                   D_8009BE78  (s16)         0x8009BE78   ALREADY PRESENT
 *   info.h                   D_8009BE7A  (s16)         0x8009BE7A   ALREADY PRESENT
 *   ctlbuf[]                 g_gpu_color_table (u8[])  0x800F189C   ALREADY PRESENT
 *   set_alarm()              func_8007DC68()           0x8007DC68   ALREADY MATCHED C (display.c:965)
 *   get_alarm()              func_8007DC9C()           0x8007DC9C   ALREADY MATCHED C (display.c:969)
 *   get_dx()                 func_8007CA00()           0x8007CA00   ALREADY MATCHED C (display.c:626)
 *   _cwc()                   gpu_StartDmaList()        0x8007D344*  ALREADY MATCHED C (display.c:786)
 *   _param()                 gpu_GetInfo()             0x8007D3A4*  ALREADY MATCHED C (display.c:795)
 *   _cwb()                   gpu_SendData()            —            ALREADY MATCHED C (display.c:778)
 *   _ctl()                   func_8007D2CC()           0x8007D2CC   ALREADY MATCHED C
 *   _getctl()                func_8007D2F4()           0x8007D2F4   ALREADY MATCHED C
 *   _otc()                   func_8007CAC8()           0x8007CAC8   ALREADY MATCHED C
 *   DR_ENV D_800E8640        D_800F1858 (see below)    0x800F1858   *** NEW DECL NEEDED ***
 *   RECT*                    GpuRect* (see below)      —            *** NEW TYPE NEEDED ***
 *   (* = call-target address read off the jal in the asm, not from a symbol file)
 *
 * NOTE ON THE STATICS THE BRIEF ASKED ABOUT:  get_alarm/set_alarm/get_dx are
 * NOT INCLUDE_ASM — they already have matching C in src/display.c.  Only
 * _clr / _dws / _drs need transplanting.  func_8007DC9C (get_alarm) is still a
 * queue item (4 regfix rules) but it is a *codegen* problem on C that already
 * exists, and psyz leaves get_alarm as INCLUDE_ASM — psyz cannot seed it.
 * Do not touch it as part of this unit.
 * ======================================================================== */

/* --- NEW TYPE 1: the GPU rectangle (psyz `RECT`) ------------------------
 * Evidence: every one of the three functions does  lh 0x4 / sh 0x4 (w),
 * lh 0x6 / sh 0x6 (h),  lhu 0x0 (x, in _clr's 0x3F alignment test),
 * lw 0x0 and lw 0x4 (the two 32-bit GPU packet words).
 * This REPLACES `typedef struct { s32 unk0; s16 x; s16 y; } _GpuChunkHdr_CE0C;`
 * at src/display.c:683-687, which is the same memory described at the wrong
 * granularity ([[split-scalars-hide-aggregate]]).  The type is local to
 * display.c; no symbol-file or asm/data change is involved. */
typedef struct {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
    /* 0x4 */ s16 w;
    /* 0x6 */ s16 h;
} GpuRect;

/* --- NEW TYPE 2: the draw-env packet aggregate (psyz `DR_ENV D_800E8640`) ---
 * *** THIS IS THE [[split-scalars-hide-aggregate]] CASE FLAGGED IN THE SWEEP. ***
 * splat named this object one-word-at-a-time: D_800F1858, D_800F185C,
 * D_800F1860, D_800F1864, D_800F1868, D_800F186C, D_800F1870, D_800F1874,
 * D_800F1878, D_800F187C, D_800F1880, D_800F1884, D_800F1888
 * (undefined_syms_auto.txt:543-555).  Those 13 names are splat output, not
 * evidence about the original object ([[splat-symbol-names-are-not-evidence]]).
 * The original is ONE aggregate: tag @ +0x00, code[0..] @ +0x04.
 *
 * WHY THE AGGREGATE STILL PRODUCES THE PER-WORD RELOCS: for a store to
 * D_800F1858.code[3], GCC 2.7.2 at -G0 emits `lui $at,%hi(D_800F1858)` +
 * `sw rt,%lo(D_800F1858+0x10)($at)`.  %hi is 0x800F and %lo is 0x1868 — the
 * SAME 8 bytes the disassembly renders as %hi/%lo(D_800F1868).  Confirmed by
 * the one place the target DOESN'T reload: `&code[8]` is taken as an address
 * (lui+addiu into $a2) and $a2 is then reused for `code[8] = 0x03FFFFFF`
 * (`sw $a3, 0x0($a2)`), which is precisely aggregate codegen and is NOT
 * reproducible from 13 independent scalars.
 *
 * WHAT THE MEASURING SESSION MUST DO:
 *   - add the `extern GpuDrEnv D_800F1858;` declaration below to src/display.c
 *   - do NOT edit asm/data/*, undefined_syms_auto.txt, symbol_addrs.txt or
 *     named_syms.txt.  D_800F1858 is a linker-script absolute symbol; a C
 *     `extern` of a wider type at the same address links unchanged, and the
 *     other 12 D_ symbols simply go unreferenced (they already are).
 *   - the size below (tag + code[15] = 0x40) is PsyQ's stock DR_ENV and fits:
 *     0x800F1858 + 0x40 = 0x800F1898, and the next named object,
 *     g_gpu_color_table, starts at 0x800F189C.  Only code[0..11] are touched
 *     by _clr, so the exact tail size is not load-bearing for the match. */
typedef struct {
    /* 0x00 */ u32 tag;
    /* 0x04 */ u32 code[15];
} GpuDrEnv;

extern GpuDrEnv D_800F1858;
