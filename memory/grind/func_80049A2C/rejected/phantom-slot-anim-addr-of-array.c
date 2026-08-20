/* REJECTED (s8, forensics) - I: p_anim = &D_800EF980[temp_v1] instead of u8* pointer arithmetic
 *
 * WHY DEAD: vars=0, 107 insns - codegen-identical; the address add still has three uses so combine cannot delete it.
 *
 * Measured with the instrumented cc1 (BB2_FRAME_DEBUG=1) on the full
 * src/text1b.c TU; artifacts in tmp/grind/func_80049A2C/s8/.
 * Target needs .frame $sp,48 (vars=8); every form here reports vars=0.
 */
