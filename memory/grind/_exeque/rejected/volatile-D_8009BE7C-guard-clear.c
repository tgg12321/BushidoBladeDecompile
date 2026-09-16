/* REJECTED this session (s2, structural) — NOT a cheat by the 6-test
 * checklist necessarily, but NOT submittable under the current frozen
 * legitimate-volatile-interrupt-touched two-prong carve-out either, so it
 * is parked here rather than committed to candidate.c.
 *
 * Measured: declaring `volatile s32 *p = &D_8009BE7C;` (in place of the
 * plain `s32 *p = &D_8009BE7C;` in candidate.c) drops sandbox --disable all
 * from 12 to 10/187 — it prevents cc1's delay-slot filler from moving the
 * `*p = 0;` store into the trailing `jalr v0` delay slot, matching target's
 * unfilled-nop shape exactly (confirmed via objdump: the entire final
 * callback block becomes byte-identical modulo masked branch targets).
 *
 * Why this is NOT submitted:
 *   - D_8009BE7C is written both by _addque2 (ordinary/non-IRQ context,
 *     src/display.c:902 `D_8009BE7C = 1;`) and by _exeque itself.
 *   - _exeque IS installed as an asynchronous DMA-interrupt callback
 *     (`DMACallback(2, _exeque);`, src/display.c:915,927) — so prong 1 of
 *     .claude/rules/legitimate-volatile-interrupt-touched.md ("G is
 *     asynchronously mutated by ... a function called from [an] installed
 *     handler") is plausibly satisfiable if _exeque itself counts as "the
 *     IRQ handler" for D_8009BE7C, since it is the function performing the
 *     read-clear on this specific execution path.
 *   - HOWEVER prong 2 requires the use-site to be one of exactly three
 *     catalogued shapes: spin-wait, double-read-across-sequence-point, or
 *     IRQ-mutated-loop-bound. This use site is a single guarded read
 *     (`if (*p != 0 && D_8009BE80 != 0) { *p = 0; callback(); }`) — a
 *     classic interrupt "test-and-clear a pending flag" idiom, but NOT
 *     textually any of the three listed shapes. The rule is explicit:
 *     "Other use-site shapes default-FAIL pending fresh SOTN research."
 *   - Per no-new-park-categories / ordinary-c-judge-decidable, a construct
 *     outside the frozen family list (or outside a sanctioned family's own
 *     documented scope) is a FAIL, not a judgment call for the worker to
 *     make — and self-authorizing a new use-site shape is exactly what
 *     [[judge-sole-gate]] rule 4 reserves for the owner via the borderline
 *     ledger, never the worker in-session.
 *
 * Disposition: banked here as a KILLED-if-submitted-under-current-rule
 * finding (see hypotheses.md H6). A future session in `escalation`
 * modality (or any session willing to spend a `ruling-request`) should ask
 * whether "IRQ-callback function does a single-read test-and-clear on a
 * flag it also owns" qualifies as a FOURTH catalogued use-site shape,
 * citing this file + the objdump evidence (candidate.c score 12 -> this
 * form's score 10). Do NOT resubmit this exact form as candidate-ready
 * without either (a) a landed ruling adding the shape, or (b) finding an
 * equivalent pure-C non-volatile structure that achieves the same
 * delay-slot placement (not found this session).
 */
s32 _exeque(void) {
    s32 mask;

    if (*D_8009BF54 & 0x01000000) {
        return 1;
    }
    mask = SetIntrMask(0);
    D_8009BF84 = mask;
    if (D_8009BF78 != D_8009BF7C && !(*D_8009BF54 & 0x01000000)) {
        do {
            if (((D_8009BF7C + 1) & 0x3F) == D_8009BF78 && D_8009BE80 == 0) {
                DMACallback(2, 0);
            }
            while (!(*D_8009BF48 & 0x04000000)) {
            }
            _que[D_8009BF7C].func(_que[D_8009BF7C].arg, _que[D_8009BF7C].count);
            D_8009BF68[0] = (s32)_que[D_8009BF7C].func;
            mask = (s32)_que[D_8009BF7C].arg;
            D_8009BF6C = mask;
            mask = _que[D_8009BF7C].count;
            D_8009BF70 = mask;
            D_8009BF7C = (D_8009BF7C + 1) & 0x3F;
        } while (D_8009BF78 != D_8009BF7C && !(*D_8009BF54 & 0x01000000));
    }
    SetIntrMask(D_8009BF84);
    if (D_8009BF78 == D_8009BF7C && !(*D_8009BF54 & 0x01000000)) {
        volatile s32 *p = &D_8009BE7C;   /* REJECTED spelling — see header */
        if (*p != 0 && D_8009BE80 != 0) {
            *p = 0;
            ((s32 (*)(void))D_8009BE80)();
        }
    }
    return (D_8009BF78 - D_8009BF7C) & 0x3F;
}
