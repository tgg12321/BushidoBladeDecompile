/*
 * REJECTED — SOTN ch[4] non-sequential write order form
 * Date: 2026-06-05
 * Rule violation: dead-vars-local-array family (thin variant);
 *                 formal borderline-research DO-NOT-SANCTION verdict.
 * Measured floor: 15 (build_insns 31 vs target 33, -2 from match).
 *                 Down from round-7 floor of 27 (-12 improvement).
 *
 * EMPIRICALLY this form is the best-scoring legitimate-shape candidate
 * to date. But the formal 2026-06-05 borderline-research survey
 * (tmp/borderline_research_array_nonseq_2026-06-05.md) found:
 *
 *   1. Only ONE SOTN matched instance (get_tw in libgpu/sys.c).
 *      The other 2 hits in src/pc/ are port-mirrors of the SAME
 *      function, not independent corroboration.
 *   2. Sibling GP0 builders in the same SOTN file (get_cs, get_ce,
 *      get_ofs) explicitly do NOT use this pattern.
 *   3. ZERO PR/issue/wiki/comment artifact — PR #1932 merged get_tw
 *      silently. No code-review acknowledgment of the array pattern.
 *      Every prior sanctioned SOTN-accepted technique had explicit
 *      acknowledgment.
 *   4. Structural family already forbidden via dead-vars-local-array
 *      (thin variant where slots are read in return rather than
 *      written-only).
 *   5. Closest existing sanctions (narrow-byte-args-packed-call,
 *      split-read-defeats-hoist) sanction SEMANTICALLY MEANINGFUL
 *      facts; this lever is write-order on a scratch buffer with
 *      no semantic content.
 *   6. Large abuse surface — N-laundering, order-laundering,
 *      naming-laundering (ch instead of pad to dodge the
 *      naming-announces-intent detector).
 *
 * Verdict: DO-NOT-SANCTION per formal methodology. The form is
 * preserved here so future agents see the result and don't re-derive
 * this avenue thinking SOTN's get_tw precedent is sufficient — the
 * adversarial research below the surface shows it isn't.
 *
 * The non-sequential write order [0],[2],[1],[3] is EMPIRICALLY
 * load-bearing for the score (sequential [0],[1],[2],[3] regresses
 * +8 to score 23). That empirical dependence on write order IS the
 * Test #3 GCC-internals-justification failure: the form's only
 * mechanism for matching is influencing cc1's scheduling shape,
 * with no source-faithful semantic explanation beyond "SOTN happened
 * to write it this way."
 */

s32 func_8007C97C(u8 *arg0) {
    u32 ch[4];
    if (arg0 != 0) {
        ch[0] = arg0[0] >> 3;
        ch[2] = (-*(s16 *)(arg0 + 4)) >> 3;
        ch[1] = arg0[2] >> 3;
        ch[3] = (-*(s16 *)(arg0 + 6)) >> 3;
        return ((u32)ch[1] << 0xF) | ((u32)ch[0] << 0xA) | 0xE2000000u | ((u32)ch[3] << 5) | (u32)ch[2];
    }
    return 0;
}
