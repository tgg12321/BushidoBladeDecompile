/* REJECTED s2 2026-07-12: structural axis (if/else-if merge of the two
   mutually-exclusive clamp checks). Measured sandbox score = 11 (build_insns
   82 vs target 87) -- target re-reads/re-checks the positive bound
   unconditionally after the negative clamp, i.e. it is NOT an else-if;
   the two ifs are independent statements. KILLED as a structural lever. */
if ((s16)g_file_vram_timer < -0x1C00) {
    g_file_vram_timer = -0x1C00;
} else if ((s16)g_file_vram_timer >= 0x7401) {
    g_file_vram_timer = 0x7400;
}
