/* REJECTED (s2, structural) — two placement variants of the pointer, both non-zero.
 *
 * (A) pointer declared at FUNCTION scope, used only in the body: sandbox = 6 (93 insns).
 *     The declaration's BLOCK-LOCAL scope is load-bearing, not just its use sites —
 *     hoisting the decl out of the if-block loses the match even though the uses are
 *     byte-identical C.
 *
 * (B) pointer declared at function scope AND used in the CONDITION too (the shape a
 *     human would most naturally write — one alias, used everywhere): sandbox = 8,
 *     92 insns (one FEWER than target).  Target reads D_8010277C symbol-direct in the
 *     condition (lui %hi / lb %lo) and only address-caches inside the block, so the
 *     condition/body asymmetry the 2026-06-22 audit called "the tell" is a FACT ABOUT
 *     THE TARGET BYTES, not an artifact of our spelling.  A uniformly-used pointer
 *     cannot match.
 */

/* (A) */
void se_data_set_A(void) {
    s32 s2 = (s32)0x80190800;
    s32 s1;
    s32 *s0;
    u8 *p = &D_8010277C;            /* <-- function scope */

    gpu_EnableDisplay();

    if (D_800A36A4 != D_800A390E
        || *(&D_8008E5A8 + (s8)D_8010277C) != D_800A30FC   /* condition: direct */
        || *(&D_8008E5A8 + D_8010277D) != D_800A30FD) {
        /* body uses (s8)*p ... */
    }
}

/* (B) */
void se_data_set_B(void) {
    s32 s2 = (s32)0x80190800;
    s32 s1;
    s32 *s0;
    u8 *p = &D_8010277C;

    gpu_EnableDisplay();

    if (D_800A36A4 != D_800A390E
        || *(&D_8008E5A8 + (s8)*p) != D_800A30FC           /* condition: via p */
        || *(&D_8008E5A8 + D_8010277D) != D_800A30FD) {
        /* body uses (s8)*p ... */
    }
}
