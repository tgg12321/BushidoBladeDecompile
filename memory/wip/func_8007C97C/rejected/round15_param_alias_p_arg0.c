/* Round 15 (2026-06-08) — REJECTED form: `u8 *p = arg0;` parameter alias
 *
 * Measured: sandbox score 25 (vs round-7 baseline 27, Δ=-2);
 *           build_insns 25 (vs baseline 24, Δ=+1).
 *
 * Cheat-reviewer verdict: FAIL on ALL SIX tests (2026-06-08).
 *
 *   Test 1 (semantic purpose): FAIL — p and arg0 are identical in type,
 *     value, every usage site. Neither is reassigned. The alias carries
 *     zero new information.
 *   Test 2 (human programmer): FAIL — a programmer writing a 7-line GP0
 *     packet builder from spec would not introduce a single-letter alias
 *     for an already-short param name. No readability gain.
 *   Test 3 (GCC-internals justification): FAIL — the mechanism by which
 *     this alias lowers the score is exclusively through RTL pseudo-
 *     creation: an extra pseudo for p at expand_function_start alters
 *     allocno priority/liveness vs round-7's single-allocno baseline
 *     (round-13 ALLOCDBG dump confirms single-allocno structure). The
 *     only honest justification cites GCC's allocator behavior.
 *   Test 4 (permuter-search-derived): FAIL — T2 was a permuter campaign
 *     proposal, not a hand-derived semantic insight. Compare round 7's
 *     u8-narrow-type lever which had semantic justification (color
 *     channels are 0..255 by domain).
 *   Test 5 (family check): FAIL — same syntactic form as
 *     [[param-local-alias-prologue-pair-flip]] (ARCHIVED FORBIDDEN
 *     2026-06-02). That rule's core test ("identical lvalues, identical
 *     types, no extra information conveyed; existence solely to
 *     manipulate cc1's allocator") applies identically here. Single
 *     vs multiple aliases is irrelevant — the rejected family is the
 *     literal-rename pattern itself, not the ordering it enabled.
 *     Also confirmed by [[drop-param-alias-local]] which describes the
 *     INVERSE direction (removing such an alias to retire cheat-asm
 *     pins) and identifies the construct as an RA-manipulation vector.
 *   Test 6 (naming-announces-intent): FAIL (marginal) — single-letter
 *     `p` in a function whose only uses are dereferences carries no
 *     semantic information over `arg0`. Stereotypical alias-only name.
 *
 * The `move p, arg0` instruction emitted is NOT a defense; the
 * [[param-local-alias-prologue-pair-flip]] precedent involved
 * prologue-emitted moves that also appeared in assembly and were still
 * forbidden. The issue is whether the SOURCE CONSTRUCT has semantic
 * purpose; it does not. Producing observable bytes via a no-semantic-
 * purpose construct is the signature of codegen-coercion.
 *
 * Floor remains 27 (round-7 candidate). Function continues INCOMPLETE.
 */
s32 func_8007C97C(u8 *arg0) {
    u8 *p = arg0;
    if (p != 0) {
        u8 r  = p[0] >> 3;
        u8 g  = (-*(s16*)(p + 4)) >> 3;
        u8 b1 = p[2] >> 3;
        u8 b2 = (-*(s16*)(p + 6)) >> 3;
        return ((u32)b1 << 0xF) | ((u32)r << 0xA) | 0xE2000000u | ((u32)b2 << 5) | (u32)g;
    }
    return 0;
}
