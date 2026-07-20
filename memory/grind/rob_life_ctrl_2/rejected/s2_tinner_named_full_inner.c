/* REJECTED s2 — named `s32 tinner = (s * (0x1000 - f)) >> 12;` computed BEFORE q.
 * Form: { s32 tinner = (s*(0x1000-f))>>12;
 *         q = (v*(0x1000-((s*f)>>12)))>>12;
 *         t = (v*(0x1000-tinner))>>12; }
 * Score: 2 (unchanged). objdump bytes IDENTICAL to probe2 — GCC CSE folds all
 * the block-local named intermediates into the same DAG. */
