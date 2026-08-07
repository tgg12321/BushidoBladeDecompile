/* s8 rederive: const-qualified base initializer.
 *   const u32 *const base = (const u32 *)arg0;
 * Sandbox --disable all: score=1 (byte-identical to baseline s3-s7).
 * GCC 2.7.2's cse2 canon_reg substitution treats const-qualified copies
 * identically to plain copies — the {reg 72, reg 78, reg 79} equivalence
 * class still forms and insn 36 still folds to reg 72 ($a0). KILLED as
 * a lever; retained here as a proven-inert stylistic variant.
 */
