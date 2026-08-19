/* ===========================================================================
 * DRAFT TRANSPLANT — func_8007CE0C  ( = PsyQ libgpu sys.c `_dws`, LoadImage )
 * 0x8007CE0C · src/display.c:689 (has C today) · dist 48 · *** 23 regfix rules ***
 *
 * (a) psyz source: tmp/psyq_prov/psyz/decomp/src/libgpu/sys.c:745-785
 * (b) Read _libgpu_sys_shared_decls.h FIRST for the symbol map + GpuRect.
 *
 * WHAT THIS REPLACES.  src/display.c:689-770 already has a body, but it is a
 * hand-built goto chassis carrying TWO cheats plus 23 regfix rules:
 *     register s32 var_s5 asm("s5");              <- register pin
 *     __asm__ volatile("" : "=r"(var_s5) : "0"(var_s5));   <- scheduling barrier
 * Both go away here.  psyz's body is the same algorithm written straight.
 *
 * MAPPING DECISIONS
 *   psyz                       BB2
 *   -------------------------  ------------------------------------------
 *   RECT *rect                 GpuRect *rect   (replaces _GpuChunkHdr_CE0C,
 *                              which modelled x/y as one s32 `unk0`)
 *   u_long *data               s32 *data       (matches the current sig)
 *   set_alarm() / get_alarm()  func_8007DC68() / func_8007DC9C()
 *   CLAMP(rect->w,0,info.w)    ternary on D_8009BE78  (NOTE: no `-1` here,
 *                              unlike _clr — the asm confirms: `slt $v0,$v0,$a1`
 *                              against the raw D_8009BE78 load)
 *   *GPU_STATUS / *GPU_DATA    *g_gpu_stat_reg / *g_gpu_data_reg
 *   *DMA2_MADR/_BCR/_CHCR      *g_gpu_dma_madr / *g_gpu_dma_bcr / *g_gpu_dma_chcr
 *   STATUS_READY_TO_RECEIVE_CMD (1<<26)   0x04000000  (BB2 psx.h has GP1_DMA_DIR
 *                              at that value but it means something else there;
 *                              display.c:939 already writes the literal — matched)
 *   CMD_CLEAR_CACHE            0x01000000
 *   CMD_COPY_CPU_TO_VRAM       0xA0000000
 *
 * (c) *** THE ONE REAL RISK — READ THIS BEFORE MEASURING ***
 *   psyz declares a plain `int var_s4;`, assigns `var_s4 = 0;` at the top, and
 *   never writes it again, then branches on it:
 *       *GPU_DATA = var_s4 ? 0xB0000000 : 0xA0000000;
 *   The target asm AGREES that the value stays live and the branch is real:
 *       8007CE3C  addu $s5, $zero, $zero        (set 0, callee-saved reg)
 *       8007CF68  beqz $s5, .L8007CF74          (real branch, ~75 insns later)
 *   But GCC 2.7.2 at -O2 normally CONSTANT-FOLDS exactly this.  BB2's previous
 *   session evidently hit that fold, which is why the current body carries the
 *   asm("s5") pin + the empty-asm barrier.  So:
 *     - MEASURE THE PLAIN FORM FIRST (it is written below verbatim).  psyz
 *       matched it against the same Sony object, so it may simply work under
 *       our cc1 — psyz builds with cc1-psx-272 + `-g`, we build decompals cc1
 *       with `-mel` and no `-g`; that delta is the likeliest explanation.
 *     - IF IT FOLDS, the pin is NOT the answer (it is the cheat we are here to
 *       remove).  Diagnostic only, never a commit: a one-off `-g` compile tells
 *       you whether the fold is flag-driven; changing CC_FLAGS is forbidden by
 *       [[no-compiler-divergence]], so a positive result there means the
 *       ORIGINAL SOURCE had a shape that blocked the fold, and the job is to
 *       find that shape.  Candidate shapes, cheapest first:
 *         i.  a third parameter `int mode` that all callers pass 0 (check every
 *             caller of 0x8007CE0C in asm/funcs for a third `$a2` set-up — if
 *             any caller sets $a2, this is settled)
 *         ii. `int var_s4 = 0;` promoted to a file-scope static in sys.c
 *             (would give a `lw`, not `addu $s5,$zero,$zero` — the asm rules
 *             this OUT, recorded so nobody re-derives it)
 *         iii. do-while(0) around the assignment ([[do-while-zero-exception]],
 *              sanctioned 2026-07-06 for any codegen effect) — the same lever
 *              already used two functions up at display.c:645 in func_8007CA00.
 *     - Do NOT reintroduce the register pin or the empty __asm__ barrier under
 *       any outcome; they are why this function is on the queue.
 *
 * (d) OTHER OPEN QUESTIONS
 *   1. psyz writes `to_write = (rect->w * rect->h + 1) / 2;` and then
 *      `% 16` / `/ 16`.  Written as division here so GCC emits its own
 *      round-toward-zero expansion (`srl 31; addu; sra 1` / `sra 5`), which is
 *      exactly what the asm shows.  Do not hand-expand it.
 *   2. `while (var_s0--)` is a post-decrement loop; the asm's
 *      `while (--remainder != -1)` in the current body is the same thing after
 *      GCC rotation.  Keep psyz's spelling.
 *   3. Retire the 23 regfix rules only after `sandbox --disable all` reads 0.
 * ======================================================================== */

s32 func_8007CE0C(GpuRect *rect, s32 *data) {
    s32 to_write;
    s32 size;
    s32 var_s0;
    s32 var_s4;

    var_s4 = 0;
    func_8007DC68();
    rect->w = rect->w < 0 ? 0 : (rect->w > D_8009BE78 ? D_8009BE78 : rect->w);
    rect->h = rect->h < 0 ? 0 : (rect->h > D_8009BE7A ? D_8009BE7A : rect->h);
    to_write = (rect->w * rect->h + 1) / 2;
    if (to_write <= 0) {
        return -1;
    }
    var_s0 = to_write % 16;
    size = to_write / 16;

    while (!(*g_gpu_stat_reg & 0x04000000)) {
        if (func_8007DC9C() != 0) {
            return -1;
        }
    }

    *g_gpu_stat_reg = 0x04000000;
    *g_gpu_data_reg = 0x01000000;
    *g_gpu_data_reg = var_s4 ? 0xB0000000 : 0xA0000000;
    *g_gpu_data_reg = *(s32 *)&rect->x;
    *g_gpu_data_reg = *(s32 *)&rect->w;

    while (var_s0--) {
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
