/* func_80052B7C — MEASURED s2 (structural). Applied to src/main/3AB48.c this
 * session and re-reverted to INCLUDE_ASM before finishing (asm-until-matched
 * rule 1: no draft C on main for an INCOMPLETE function). sandbox
 * func_80052B7C --disable all --diff on this exact body: score 16 (down from
 * the s1 floor of 24), build_insns == target_insns == 24, all 5 remaining
 * hunks classed source-level (no reg-alloc/scheduling lever can close them).
 *
 * Needs: #include <psxsdk/inline_c.h> added to src/main/3AB48.c (NOT
 * inline_o.h — see finding below, inline_c.h's macros use the pointer
 * argument directly as the asm base register with no staging "move",
 * which is what drops build_insns from 27 (inline_o.h) to 24 (inline_c.h)).
 *
 * Signature (from asm/funcs/func_80052B7C.s, unchanged from s1 recon):
 *   a0 -> MATRIX*  (5 lw -> ctc2 CR0-CR4, matches MATRIX.m[3][3]+pad)
 *   a1 -> SVECTOR* (3 lh -> ctc2 CR5-CR7, sign-extended halfwords; no
 *                   vendored macro loads TR from halfwords -- written
 *                   island, cites inline_asm_canonical.txt:340)
 *   a2 -> VECTOR*  (2 lwc2 -> DR0/DR1, matches gte_ldv0 exactly)
 *   a3 -> VECTOR*  (3 swc2 <- DR9/DR10/DR11, matches gte_stlvl's STORES
 *                   exactly, but NOT the trailing jr-delay-slot placement
 *                   -- see finding 2 below)
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

/* ---- s2 findings (both are NEW evidence beyond s1's manual macro-text read) ----
 *
 * Finding 1 (chunk 1, gte_SetRotMatrix): s1's evidence.md claimed this macro's
 * asm body is "byte-identical" to the target's chunk 1. That claim is WRONG at
 * the instruction-ORDER level (it was a manual text comparison, never built).
 * BOTH vendored headers' gte_SetRotMatrix interleave 2-lw+2-ctc2, then
 * 3-lw+3-ctc2 (inline_o.h additionally stages the pointer into $12 first,
 * costing 3 extra `move`/register-reuse insns -- that is finding-1a, already
 * fully explained by inline_o.h vs inline_c.h header choice and closed by
 * switching headers). But NEITHER header's gte_SetRotMatrix reproduces the
 * target's actual order: target does all 5 `lw` first, THEN all 5 `ctc2`
 * (asm/funcs/func_80052B7C.s lines 1-5 then 9-13). No finer-grained vendored
 * macro exists to compose that batched order from smaller pieces (grepped
 * inline_c.h for any standalone CR0-4 ctc2-only macro: none). So chunk 1, like
 * chunk 2, has NO vendored-macro-call spelling that reproduces the target's
 * exact instruction order -- this is new evidence for the existing frontier
 * item 3 ruling-request, not just chunk 2's problem.
 *
 * Finding 2 (chunk 5, gte_stlvl): target's last `swc2 $11,8(a3)` sits in the
 * `jr $ra` delay slot (asm/funcs/func_80052B7C.s line 24). Our build (both
 * header choices) emits `swc2 $11,8(a3)` BEFORE `jr $ra`, with an explicit
 * `nop` in the delay slot instead. Mechanism (not yet dump-confirmed --
 * `tools/grinder/dump.ps1 func_80052B7C` produced no `.rtl`/`.sched` file this
 * session, see artifacts): gte_stlvl's asm template bundles 3 `swc2`
 * instructions into ONE volatile asm statement; GCC's delay-slot filler
 * (reorg.c) can steal a single trailing ordinary insn for a jr's delay slot,
 * but a multi-instruction inline-asm block is scheduled as one opaque unit it
 * does not reach inside of. No vendored macro exists that stores only
 * DR9-11's last word alone (to let it be the trailing statement by itself),
 * so this is structurally unreachable via a gte_stlvl macro CALL as currently
 * defined in either header -- splitting it into 3 separate single-swc2 asm
 * statements would fix it but is a WRITTEN-OUT COPY of an existing macro's
 * body, which the current GTE-macro-calls policy (inline-asm-policy.md
 * "Calls, not copies") disfavors when a macro already exists for the op.
 *
 * Both findings sharpen (don't replace) the existing frontier item 3
 * ruling-request: it is not just chunk 2 (no macro at all) that may need a
 * written-island carve-out -- chunks 1 and 5 need one too, for a DIFFERENT
 * reason (a macro exists, but its bundled body can't reproduce the target's
 * instruction order/delay-slot placement). The ruling-request question should
 * cover both reasons.
 */
