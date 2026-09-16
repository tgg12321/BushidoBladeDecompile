/* REJECTED s6 — rewriting the field28 dispatch as a FLAT if/else-if chain in
 * target's literal runtime compare order (==3 first, <4 second, ==4 third —
 * read directly off asm/funcs/func_8006CCC8.s:82-98's beq/slti/beq sequence)
 * regressed the honest floor from 20 to 54 on the array-index-addressing
 * chassis (before the for-loop update-clause reorder that later reached 18).
 *
 *     if (field == 3) {
 *         if (*arg1 & (0x40 << shift)) { ... +0x1A loop ... }
 *     } else if (field < 4) {
 *         if (field >= 0 && (*arg1 & (0xF0 << shift))) { ... default ... }
 *     } else if (field == 4) {
 *         if (*arg1 & (0x40 << shift)) { ... +0x1D loop ... }
 *     }
 *
 * measured_on: src/text1b.c HEAD s6, floor-20 chassis (array-index address
 * fix applied); sandbox --disable all: score 54, build_insns 188.
 *
 * The nested form (`if (field != 4) { if (field != 3) { if (field < 4) {
 * default } } else { case3 } } else { case4 }`, candidate.c's actual shape,
 * first established in s4) tests field==4 FIRST at the RTL/asm level despite
 * NOT matching target's literal source-order compare sequence -- confirming
 * again (as s4's own evidence already showed) that GCC 2.7.2's block-layout
 * convention for C-level nested if/else does not preserve the intuitive
 * "outer-to-inner compares in written order" mental model target's own
 * asm suggests; matching the physical BLOCK LAYOUT (which arm falls through,
 * which is a forward jump) is the lever that works here, not matching the
 * literal compare sequence. Side-probed the mirror nested form (outer test
 * `field != 3` instead of `field != 4`, same physical block layout) on the
 * SAME floor-18 chassis: scored an IDENTICAL 18 (neutral, not saved as a
 * separate rejected form) -- so the two nested-outer-test choices are
 * interchangeable on this chassis; candidate.c keeps `field != 4` outer for
 * continuity with the s4 ledger lineage.
 */
