/* REJECTED (s44, synthesis / SIBLING SWEEP). Axis B, seat probe.
 * Transplant of CD_sync/CD_datasync's sanctioned pointer-alias-to-a-global
 * device (void **pp = &D_800F19C0;) onto the stat-register pointer global,
 * to change WHICH pseudos exist across the dead read's live range:
 *   volatile u32 **statp = &g_gpu_stat_reg;
 *   printf(&g_str_gpu_timeout, ..., **statp, *g_gpu_dma_chcr, *g_gpu_dma_madr);
 * Measured: score 17, build_insns 92 -- STRICTLY WORSE. The extra indirection
 * materialises as real instructions instead of merely reshaping local-alloc's
 * quantity table. The companion probe on the madr pointer global
 * (u32 **madrp = &g_gpu_dma_madr; ... **madrp) measured 9 / build 90: inert,
 * no seat change, banked in hypotheses.md. */
