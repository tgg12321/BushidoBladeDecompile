/* REJECTED (s8, forensics) - N: form a SECOND pointer &D_80099D3C[k+3] for the last three reads
 *
 * WHY DEAD: vars=0 - both pointers have multiple uses, so neither address add is single-use and neither is deleted by combine.
 *
 * Measured with the instrumented cc1 (BB2_FRAME_DEBUG=1) on the full
 * src/text1b.c TU; artifacts in tmp/grind/func_80049A2C/s8/.
 * Target needs .frame $sp,48 (vars=8); every form here reports vars=0.
 */
