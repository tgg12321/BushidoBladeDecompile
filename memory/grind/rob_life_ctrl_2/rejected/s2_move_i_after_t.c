/* REJECTED s2 — move `i = h >> 12;` AFTER the p/q/t block (LUID perturbation).
 * Form: h*=6; f = h&0xFFF; q = ...; t = ...; i = h>>12;
 * Score: 2 (unchanged). objdump bytes IDENTICAL to HEAD. Declaration/definition
 * position for `i` is invisible to codegen here — GCC reorders the shifts freely.
 */
