/* REJECTED (s2, structural) — three &-free re-associations of the direct-cast
 * body. None improves on the plain direct spelling (6); two are much worse.
 * Together with rejected/direct-cast-no-address-cache-scores-6.c and
 * rejected/value-staging-local-scores-9.c, this closes out the STRUCTURAL sweep
 * of the &-free subspace: 6 measured forms, floor of the whole subspace = 6.
 *
 *   V06 subscript spelling   (&D_8008E5A8)[i]  instead of *(&D_8008E5A8 + i)  = 6
 *        Ties V01 exactly, as predicted: same RTL. Spelling-invariance control —
 *        confirms the subspace map is about STRUCTURE, not surface syntax.
 *   V02 store re-association: the two table refreshes moved AHEAD of the
 *        D_800A390E store                                                     = 17
 *        The direct form's gap is NOT a scheduling/ordering artifact — perturbing
 *        the store order makes it strictly worse, so 6 is a genuine floor and the
 *        missing 6 insns are purely the address-materialization shape.
 *   V03 call args hoisted into block-local u8 temps (c, d) before the calls     = 24
 *        Also grows the function to 95 insns (target 93): the temps get spilled
 *        across the four calls. Block-local var splitting is the wrong direction
 *        here — the target RE-READS both bytes after the call; it does not carry
 *        the values across it.
 */

/* V06 — subscript spelling (6) */
void se_data_set_v06(void) {
    /* ... body identical to direct-cast, with (&D_8008E5A8)[(s8)D_8010277C]
     * and (&D_8008E5A8)[D_8010277D] at all four sites ... */
}

/* V02 — store re-association (17) */
void se_data_set_v02(void) {
    /* ... direct-cast body, but:
     *     D_800A30FC = *(&D_8008E5A8 + (s8)D_8010277C);
     *     D_800A30FD = *(&D_8008E5A8 + D_8010277D);
     *     D_800A390E = D_800A36A4;        <-- moved LAST
     */
}

/* V03 — call args hoisted into block-local temps (24, 95 insns) */
void se_data_set_v03(void) {
    /* ... direct-cast body, but the two func_8005BA8C table args become:
     *     u8 c = *(&D_8008E5A8 + (s8)D_8010277C);
     *     u8 d = *(&D_8008E5A8 + D_8010277D);
     *     ... four calls ...
     *     s1 = func_8005BA8C(s2, D_800A36A4, c, d);
     */
}

/* Full sources as swept: tmp/grind/se_data_set/s2/variants/{v06,v02,v03}*.c */
