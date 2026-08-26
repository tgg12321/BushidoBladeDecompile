/* REJECTED (s7, 2026-08-26) -- score 12, 69/72 insns.
 * Identical to the matching s7 form EXCEPT the 0x80 CdRead mode is passed as a
 * literal at both func_800372F4 call sites instead of living in a local:
 *     v0 = func_800372F4(0x800, (u32 *)sp_buf, 0x80);
 *     v0 = func_800372F4(dest[3], (u32 *)dest[2], 0x80);
 * GCC rematerialises `li $a2,0x80` in each call's delay slot rather than holding the
 * value in a callee-saved register, so the function loses 3 instructions (69 vs 72)
 * and $s4 is never allocated. The target's `addiu $s4,$zero,0x80` in the prologue,
 * live across the whole retry loop, is how GCC materialises a USER LOCAL -- so the
 * original source held this argument in a variable. Re-measured on the s7 chassis;
 * supersedes the older final-call-literal-const-score10.c measurement.
 */
