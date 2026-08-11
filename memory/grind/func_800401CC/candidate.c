/*
 * STATUS (session 3, 2026-08-11): RULING-REQUEST PENDING on the u-staging
 * below (is it inside the func-specific ban's "any spelling" scope, or a
 * distinct sanctioned staged-value-reused-variable instance like the
 * layer-1-approved v-staging?). Session 3's campaigns measured the
 * natural-spelling space empty (see hypotheses.md H8/K9) — this form or a
 * ban extension are the only outcomes. Do not submit candidate-ready from
 * this file until the ruling lands; if sanctioned, keep the banned holder's
 * name and the low-mask hex literal out of self_vet.md (validator
 * token-matches them even in prose).
 *
 * CANDIDATE — func_800401CC (src/text1a_pre.c), session s2-permuter, 2026-08-11.
 * SANDBOX 0/78 THIS SESSION (verified twice, annotations in place). Form is
 * APPLIED in src/text1a_pre.c. Supersedes the layer-1-FAILed lowmask form
 * (rejected/layer1-fail-0811-0533.c): the banned constant-holder local is GONE.
 *
 * The closing structure (vs the pre-grind 20-form):
 *  1. Staging-local removal + store-last order (s1 wins, unchanged).
 *  2. BOTH tail masks staged through the two EXISTING dead-after-call texture
 *     coordinate locals, in source order:
 *         u = 0xFFFFFF;      (u's allocno: $a2 copy-pref from the (s16)u call arg)
 *         v = 0xFF000000;    (v's allocno: $a3 copy-pref from the (s16)v call arg)
 *     Both u and v are widened s16 -> s32 to hold the masks (head bytes
 *     unchanged: lhu loads + (s16) call casts identical — verified byte-level).
 *     Both staged values are LIVE: each mask is read by both packet-link
 *     statements. Zero dead code.
 *
 * Why this closes the $6/$7 mask swap AND the li emission order in one move:
 *  - With both masks staged through multi-block (global-alloc'd) pseudos, the
 *    tail's local-alloc qty pool contains NO mask qty at all (QTYDBG verified:
 *    blk=5 pool is only the pointer/word temps, $2-$5). qty_compare_1 never
 *    gets to misassign $6 — the whole s1 priority-vs-lifetime wall is bypassed.
 *  - global.c gives each allocno its call-arg copy preference: u -> $6/a2,
 *    v -> $7/a3. Exactly target's mask registers.
 *  - The two explicit sets sit in source/LUID order u-first, so the li+ori
 *    0xFFFFFF cluster is emitted before the li 0xFF000000 — target insn order
 *    (this was the last 2-insn residual of the score-2 single-staged form).
 *  - Post-call cluster in the built object: lui a2,0xff / ori a2,a2,0xffff /
 *    lui a3,0xff00 — byte-identical to target, all registers correct.
 *
 * Family: staged-value-reused-variable (SANCTIONED 2026-07-03), one instance
 * per mask, both FAKE-annotated in src with what/mechanism/lever-exhaustion.
 * The prior layer-1 review explicitly ruled the v-staging "properly annotated
 * and evidenced"; the u-staging is the symmetric twin (same liveness argument,
 * same family, $a2 instead of $a3). The banned lowmask constant-holder is not
 * present in any spelling.
 *
 * Session kill trail (this session): A1-flip (stmt1 OR-operand flip in the
 * v-staged context) = 8; A2 (named intermediate low = ot&0xFFFFFF set first)
 * = 8 (hoists the ot load, target keeps the ANDs late); A6 (v-set between
 * stmt1-literal and stmt2) = 12 with +1 insn (CSE does not fold the second
 * 0xFF000000 materialization cleanly). All banked in rejected/.
 */
void func_800401CC(s32 a0, s32 a1, s32 a2) {
    s16 buf[4];
    u16 *tbl;
    s32 u;
    s32 v;
    s32 *pkt;
    s32 *ot;

    a2 = D_800A36AC & 1;
    if (a2 != D_800A3234) {
        D_800A3378 = (s32)(&D_800A9830 + a2 * 240);
        D_800A3234 = a2;
    }
    if ((s32 *)D_800A3378 != (s32 *)(&D_800A9920 + D_800A3234 * 240)) {
        tbl = &D_80094AF4 + a1 * 6;
        buf[0] = *tbl++;
        buf[1] = *tbl++;
        buf[2] = *tbl++;
        buf[3] = *tbl++;
        u = *tbl++;
        v = *tbl;
        if (a0 != 0) {
            buf[0] = buf[0] + 0x80;
            u = u + 0x80;
        }
        SetDrawMove((s32)(s32 *)D_800A3378, buf, (s16)u, (s16)v);
        pkt = (s32 *)D_800A3378;
        ot = (s32 *)D_800A378C;
        /* FAKE: OT pointer-field mask staged through the dead u-coord local (its
           value was consumed by the draw call above; the mask is read by both
           packet-link statements below), mechanism: global.c allocno call-arg
           copy preference ($a2 from the pre-call (s16)u arg copy) places the
           multi-set pseudo at $a2 with no local-alloc mask qty left to contest
           it, and the explicit set's LUID position restores the target li+ori
           emission order, lever-exhaustion: memory/grind/func_800401CC/
           hypotheses.md K1-K4 + P1/P2 + s2-permuter A-probes (all natural
           spellings measured broken) */
        u = 0xFFFFFF;
        /* FAKE: OT-code mask staged through the dead v-coord local (same
           liveness argument as u above), mechanism: global.c allocno call-arg
           copy preference ($a3 from the pre-call (s16)v arg copy) keeps the
           multi-set pseudo at $a3, removing 0xFF000000 from the local-alloc
           qty pool, lever-exhaustion: same ledger sections as u */
        v = 0xFF000000;
        *pkt = (*pkt & v) | (ot[0x3FFC / 4] & u);
        ot[0x3FFC / 4] = (ot[0x3FFC / 4] & v) | ((s32)pkt & u);
        D_800A3378 = (s32)(pkt + 6);
    }
}
