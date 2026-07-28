/* REJECTED s2: score 16 (vs floor 6). Split-init on disc:
 *   s32 disc = arg2 * arg2;
 *   disc += arg3 * dy2;
 * Predicted to put the first mflo directly into disc's pseudo and evict dy
 * from $v1 — instead it reorders the mult launch sequence (target launches
 * mult s3,s3 into the bnez delay slot from the combined expression) and
 * diverges by +10. The combined `arg2*arg2 + arg3*dy2` expression is right. */
