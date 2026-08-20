/* REJECTED (s8, forensics) - P/Q/R: make the `8` constant holder single-use, or remove it entirely
 *
 * WHY DEAD: vars=0 in all three. R (remove it entirely) is codegen-IDENTICAL: sandbox 12, 126 insns - so the constant holder was pure noise and is now dropped from candidate.c.
 *
 * Measured with the instrumented cc1 (BB2_FRAME_DEBUG=1) on the full
 * src/text1b.c TU; artifacts in tmp/grind/func_80049A2C/s8/.
 * Target needs .frame $sp,48 (vars=8); every form here reports vars=0.
 */
