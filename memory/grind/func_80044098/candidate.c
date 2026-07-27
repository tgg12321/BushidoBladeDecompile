/* func_80044098 — s4 best form (pA): sandbox --disable all = 0 (26/26 insns),
 * measured 2026-07-27 with this form in src/text1a_c.c. PENDING OWNER RULING —
 * do NOT land as COMPLETED-C until the owner rules on the pair.
 *
 * Found by permuter (base-dw0-random campaign, output-0-1, 47 s after seed as
 * dw0+pair; decomposed in sandbox: pair alone = 0, dw0 alone = 13, exact find
 * = 0). The ONLY added construct vs the canonical base do-while form is the
 * in-loop cancellation pair `v1++; v1--;` after the real `v1++`.
 *
 * Mechanism (instrumented cc1, BB2_ALLOC_DEBUG, s4/pA_tu.allocdbg.txt):
 *   pointer pseudo 74: nrefs 12 -> 20 (+4 pair refs x2 loop weight),
 *   pri 21176 -> 47058 > counter 26250 -> pointer allocates first -> $v1,
 *   counter -> $a0; -1 const $a1 and a6 $a2 unchanged. All five regs target.
 *   The pair survives to flow because cse cannot fold a cross-statement
 *   ++/-- chain (unlike the s2 same-path decorations); post-flow combine
 *   collapses it to the single real `addiu v1,v1,4`, and refs are frozen
 *   post-flow (s2 frontier-2), so the emitted bytes are exactly target's.
 *
 * WHY NOT LANDED: the s1 Judge constraint (BINDING) declares the
 * fabricated-constant cancellation/re-set-chain spelling UNSANCTIONED for
 * this function ("v1 += 2; v1 -= 1; and any same-register re-set chain whose
 * constants are an invented decomposition of the semantic op") and directs:
 * do not respell; surface to the owner as a proposed new sanction with
 * mechanism evidence. This file + the s4 outcome JSON are that surfacing.
 *
 * Sanctioned-family alternatives for the same ref-lift, measured dead (s4):
 *   - v1 = v1; self-assign in-loop (dead-store-fake-exception): INERT, 13
 *     (no-op move discarded before flow counts refs)
 *   - do{}while(0) wrapper alone: INERT, 13 (scales both pseudos' in-loop
 *     refs; counter still wins on livelen)
 *   - dead store to fresh local: flow deletes dead insns BEFORE counting
 *     (s3 evidence, pV/pX) — cannot lift refs
 *   - duplicated-statement-into-arms: no arms exist in-loop; the peel
 *     spelling of duplication is the s2/s3 family, proven 3-locked
 * Previous best sanctioned form: pY (peel+hdr-borrow, floor 3 @ 26/26) —
 * in this file's git history (s3 version).
 */
void func_80044098(s16 a0) {
    s32 *v1;
    s32 a4;
    s32 *a6;

    v1 = D_80103608[a0];
    a4 = *(v1 - 1);
    a6 = v1 - 1;
    if (a4 & 0x8000) {
        a4 = a4 & 0x7FFF;
        *(v1 - 1) = a4;
        a4 = a4 - 1;
        if (a4 != -1) {
            do {
                *v1 -= (s32)a6;
                v1++;
                a4--;
                v1++;
                v1--;
            } while (a4 != -1);
        }
    }
}
