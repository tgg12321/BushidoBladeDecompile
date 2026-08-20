/* REJECTED (s8, forensics) - H: hoist the rotation index into a named local (kidx) without changing the access form
 *
 * WHY DEAD: vars=0, 107 insns - a named index alone changes nothing; the trigger needs a SECOND distinct index expression on the same symbol.
 *
 * Measured with the instrumented cc1 (BB2_FRAME_DEBUG=1) on the full
 * src/text1b.c TU; artifacts in tmp/grind/func_80049A2C/s8/.
 * Target needs .frame $sp,48 (vars=8); every form here reports vars=0.
 */
