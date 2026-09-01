/* REJECTED — session 2 (recon, 2026-08-31): combined-&& tail spelling.
 *
 *     y = *(s32 *)(obj + 0x108);
 *     if (y - x <= max_y && y + x >= min_y) return 1;
 *     return 0;
 *
 * (rest of body identical to candidate.c)
 *
 * Hypothesis was: both jump-if-false branches target the same else label from
 * RTL expansion (LABEL_NUSES=2), which should block jump.c's store-flag
 * if-conversion on the final diamond.
 *
 * MEASURED: sandbox --disable all -> score 8, build 119 vs target 120.
 * The fold STILL fired on the second condition (emitted `slt; xori v0,v0,1`
 * at the tail), and the first compare's register shifted from $v0 to $v1
 * (build used `slt $v1,$a3,$v1; bnez $v1` where target has $v0).
 * CONCLUSION: jump.c's store-flag conversion is robust to a multi-use
 * else-label — it rewires the last branch to the epilogue and folds anyway.
 * The multi-use-label attack on this fold is DEAD. Verdict: KILLED.
 * Worse than v1 plain early-return (3/118); do not re-propose.
 */
