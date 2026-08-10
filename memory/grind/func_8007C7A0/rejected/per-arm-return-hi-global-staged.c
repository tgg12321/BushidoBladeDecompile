/* s6 REJECTED (measured worse, not a cheat): per-arm returns while KEEPING
 * the 12-form's pre-dispatch `hi = arg1 & 0xFFF` global + L2 hi-staging of the
 * Y sign check (P2 probe): sandbox 14, build 53.
 *   - narrow arm's `ym2 = hi << 12` reads the GLOBAL hi, so the scheduler
 *     sank the sll into the jump delay slot, leaving the const's range
 *     uncovered -> narrow-arm const took $v1 while wide-arm const took $a0;
 *     the asymmetric tails defeat cross-jump -> +2 insns.
 *   - hi (fused sxt(y)+mask pseudo) landed $a0; ylim-save $a2 (both wrong).
 * DIAGNOSTIC VALUE: this probe was the first form ever to land carrier=$a3 +
 * xlim-save=$a2 + lo=$v0 with a clean structure, proving the arm-local
 * occupation mechanism. Fixed by making BOTH arms compute their masks
 * per-arm from arg1 and dropping the hi staging entirely (candidate.c, 0/51).
 */
