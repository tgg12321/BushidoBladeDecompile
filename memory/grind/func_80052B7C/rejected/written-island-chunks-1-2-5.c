/* REJECTED s3 (structural). This is the s2/s3 candidate body banked for the
 * ruling-request filed at the end of s2/s3: it uses vendored macro calls for
 * chunks 3/4 (gte_ldv0/gte_rt, confirmed byte-identical) and a WRITTEN
 * __asm__ ISLAND for chunk 2 (no vendored macro exists for a TR-from-
 * halfwords load into CR5-7), accepting chunks 1 and 5 as imperfect (macro
 * calls that don't reach the target's instruction order / delay-slot
 * placement -- see hypotheses.md s2 findings 1 and 2).
 *
 * RULING (docs/grind/decisions.md, 2026-10-15 20:38, func_80052B7C): FAIL.
 * Written __asm__ islands reproducing or splitting an existing/nonexistent
 * vendored macro's body are NOT admissible under the inline_asm_canonical.txt:340
 * grant, in any chunk, regardless of whether a macro exists for the operation
 * or whether an existing macro's bundled body can reach the target bytes.
 * Citation: .claude/rules/inline-asm-policy.md:55-57 ("Calls, not copies...
 * No `__asm__` spelling of a header macro") and :70-72 ("written-out island
 * routes... admit nothing new"). Banked here as a dead form, not for
 * resubmission in any respelling of the same construct.
 *
 * Measured (s2, re-verified s3): sandbox func_80052B7C --disable all --diff
 * on this exact body = score 16 (down from the INCLUDE_ASM floor of 24),
 * build_insns == target_insns == 24, all 5 hunks source-level. Did NOT reach
 * distance 0 even before the ruling (chunks 1 and 5 still diverged).
 */
void func_80052B7C(MATRIX *rot, SVECTOR *tr, VECTOR *v0, VECTOR *out) {
    gte_SetRotMatrix(rot);
    __asm__ volatile(
        "lh     $13, 0(%0)\n"
        "lh     $14, 2(%0)\n"
        "lh     $15, 4(%0)\n"
        "ctc2   $13, $5\n"
        "ctc2   $14, $6\n"
        "ctc2   $15, $7\n"
        :: "r"(tr) : "$13", "$14", "$15");
    gte_ldv0(v0);
    gte_rt();
    gte_stlvl(out);
}
