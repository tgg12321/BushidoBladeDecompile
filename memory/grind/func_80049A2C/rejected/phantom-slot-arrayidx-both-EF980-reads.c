/* REJECTED (s8, forensics) - B/C: array-index the D_800EF980 reads instead of holding p_anim
 *
 * WHY DEAD: vars=0 - CSE merges the two same-index reads into one pointer, so no second address add exists to orphan.
 *
 * Measured with the instrumented cc1 (BB2_FRAME_DEBUG=1) on the full
 * src/text1b.c TU; artifacts in tmp/grind/func_80049A2C/s8/.
 * Target needs .frame $sp,48 (vars=8); every form here reports vars=0.
 */
