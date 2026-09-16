/* REJECTED — s4 permuter campaign 1 (output-715-1/765-1/915-1), _exeque.
 *
 * The permuter found that marking D_8009BF6C and/or D_8009BF70 volatile
 * (`extern volatile int D_8009BF70;`, `extern volatile char D_8009BF6C;`,
 * `extern volatile short D_8009BF70;` — three separate width variants, all
 * scoring between the unwrapped baseline (980) and the legitimate
 * do-while(0) find (615)) improves the permuter's raw score. NOT applied to
 * src/display.c; rejected without needing a sandbox measurement.
 *
 * Why rejected:
 *   1. legitimate-volatile-interrupt-touched's two-prong gate requires an
 *      identifiable IRQ/MMIO writer independent of the reading function's
 *      own control flow, at one of three catalogued use-site shapes
 *      (spin-wait / double-read-across-sequence-point /
 *      IRQ-mutated-loop-bound). D_8009BF6C/D_8009BF70 are plain
 *      "last-run debug record" scalars written ONLY by _exeque itself
 *      (and _addque2's own fast-path duplicate of the same store) — there
 *      is no async writer distinct from the reader, and the use-site here
 *      is a plain sequential store, not one of the three shapes.
 *   2. This ledger already banked contradicting evidence at s1:
 *      volatile_extern_allowlist.txt:75 (the D_8009BF68 grant) explicitly
 *      states "the SAME printf folds the non-volatile adjacent siblings
 *      D_8009BF6C and D_8009BF70" — i.e. the project's own prior grant
 *      documents these two globals as the NON-volatile control case.
 *      Marking them volatile now would directly contradict that banked
 *      finding without new evidence.
 *
 * Original diff (illustrative, one of three width variants):
 *
 *   -extern s32 D_8009BF70;
 *   +extern volatile int D_8009BF70;
 */
