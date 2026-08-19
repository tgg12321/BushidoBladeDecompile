/* REJECTED — s31 permuter chassis-8 (goto/nested-if control-flow topology).
 *
 * WHY DEAD: This is the 8th structurally-distinct permuter chassis and the
 * FIRST to seed from a restructured control-flow topology (goto/nested-if +
 * hoisted vsync_lim + split draw-count increment) instead of the HEAD
 * comma-expression `||` body that all 7 prior chassis used. It preserves
 * short-circuit semantics exactly (see trace in s31 evidence).
 *
 * The restructured pre-branch region emits a leaner 88-insn body (vs the
 * proven 90-insn comma-form) that DIVERGES from target's pre-branch codegen,
 * so the masked-Levenshtein base_score is 1110 — a DISJOINT, higher-distance
 * basin ~480 above the ~630 floor basin. Over 16,934 iterations (-j8) the
 * campaign plateaued at 955 and NEVER approached the 630 floor, let alone
 * score 0. Every sub-base find is a coercion cheat:
 *   - output-955-*: `extern volatile int D_8009BF78` / `volatile s32 D_8009BF7C`
 *     volatile-coercion (stripped by engine.volatile_cheats)
 *   - output-995-*: `extern volatile long D_8009BF68[]` (banked volatile-BF68
 *     axis-A coercion, same family as s4/s5/s13/s14/s22)
 *   - output-960-1: `s32 *new_var = &D_80016044; debug_printf(new_var, ...)`
 *     dead pointer-alias junk (cheat-by-spelling)
 * D_8009BF68[0] stayed inline combine-folded in every legitimate find (axis A
 * never materialized); the fmt-vs-deadread 8-op sched cluster never
 * legitimately reordered (axis B never moved).
 *
 * CONCLUSION: even seeding the permuter from a fundamentally different
 * control-flow topology does not open a path to target — the goto basin is
 * disjoint from and farther than the comma-expression floor basin, and yields
 * only the same coercion families. Permuter now EIGHT-chassis confirmed dead
 * (~206k cumulative iters). Corroborates s8's finding that axis A is
 * per-expression (control-flow-insensitive) and axis B is volatile-order-locked
 * (block-structure-insensitive): a control-flow reshuffle changes neither.
 */
s32 func_8007DC9C(void) {
    s32 temp_v0;
    s32 count;
    s32 vsync_lim;

    vsync_lim = sys_VSync(-1);
    if (g_gpu_vcount >= vsync_lim) {
        count = g_gpu_draw_count;
        g_gpu_draw_count = count + 1;
        if (count <= 0xF0000) {
            return 0;
        }
    }
    {
        s32 dead = *g_gpu_stat_reg;
        debug_printf(&g_str_gpu_timeout, (D_8009BF78 - D_8009BF7C) & 0x3F, *g_gpu_stat_reg, *g_gpu_dma_chcr, *g_gpu_dma_madr);
        (void)dead;
    }
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
