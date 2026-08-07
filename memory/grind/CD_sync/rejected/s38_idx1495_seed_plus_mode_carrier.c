/* s38 P3: h5 base + `v0 = *idx_1495` (s35 P1 seed) + `u8 mode = *idx_1495;`
 * hoist at src/system.c:456 callback site (as a legitimate refactor —
 * reading the callback arg into a local before passing).
 * Result: masked=3 INERT vs P2 (idx_1495 seed alone, masked=3). cse.c copy-
 * propagation folds the single-use `mode` local back to *idx_1495 direct
 * read; the reg web is unchanged.
 *
 * KILLED: Frontier #2 axis (b) — named local at downstream callback site is
 * cse-transparent, does NOT participate in a distinct callee-save allocation
 * and does NOT absorb the +1 register diff. Any pointer/value carrier at
 * the callback site that would land in a distinct s-reg would have no
 * semantic purpose beyond changing register assignments (cheat family per
 * no-new-park-categories cheats-by-any-spelling).
 */
if (D_800A11B8 != 0)
{
    u8 mode = *idx_1495;
    ((void (*)(u8, void *)) D_800A11B8)(mode, &D_800F19A8);
}
