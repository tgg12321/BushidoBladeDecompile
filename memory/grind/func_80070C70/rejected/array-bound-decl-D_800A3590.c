/* REJECTED (s2, 2026-09-10) - instance kill, measured 101 on the floor-101 chassis; objdump still
 * shows `9368: addiu s3,s3,0 / R_MIPS_LO16 D_800A3590`, i.e. the hoisted `la` survives.
 *
 * WHY IT IS DEAD: giving the array a complete type does not change get_inner_reference's scale-2
 * offset, so the address still expands symbol-first and the symbol is still force_reg'd into the
 * loop-invariant pseudo that loop.c hoists.
 */
extern s16 D_800A3590[64];      /* also tried [1]; both declaration sites changed together */
