/* REJECTED (s8, forensics) - K/O: fold the D_80099CC8 base+index into one expression
 *
 * WHY DEAD: vars=0 (O also loses 2 insns) - the first block has no preceding insn boundary and only one index into that symbol.
 *
 * Measured with the instrumented cc1 (BB2_FRAME_DEBUG=1) on the full
 * src/text1b.c TU; artifacts in tmp/grind/func_80049A2C/s8/.
 * Target needs .frame $sp,48 (vars=8); every form here reports vars=0.
 */
