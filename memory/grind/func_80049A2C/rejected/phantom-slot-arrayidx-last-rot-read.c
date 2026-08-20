/* REJECTED (s8, forensics) - E/F5: spell the final rotation-table read as D_80099D3C[k+5] with the walking pointer for the rest
 *
 * WHY DEAD: vars=0 - CSE rewrites the indexed access as src[1] because src is already live; the address add is never re-formed.
 *
 * Measured with the instrumented cc1 (BB2_FRAME_DEBUG=1) on the full
 * src/text1b.c TU; artifacts in tmp/grind/func_80049A2C/s8/.
 * Target needs .frame $sp,48 (vars=8); every form here reports vars=0.
 */
