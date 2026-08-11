/* func_80047A90 — BEST FORM, session 3 (structural), honest sandbox floor 6 (down from 8).
 * Session 4 (permuter) re-verified floor 6 and left this form UNCHANGED: two
 * campaigns (~127k iters, array-index chassis + pointer-walk chassis, random +
 * directed PERM over loop-1 spellings) found nothing below the floor attractor —
 * three distinct loop-1 spellings all score identically. The residue is
 * find_reg-internal; next axis is forensics (see hypotheses.md s5 frontier).
 * Session 5 (permuter) re-verified floor 6 and left this form UNCHANGED: two
 * further campaigns (s5A loop-2+init unlocked with loop 1 locked, 26,838 iters;
 * s5B whole-function randomization, 27,904 iters) found ZERO sub-floor forms —
 * the permuter has now mutated every region of the function across s4+s5
 * (~182k iters) and only ever reaches the score-50 attractor. Permuter modality
 * exhausted; the remaining axis is forensics (BB2_ALLOC_DEBUG find_reg trace).
 * NB: s5 was the FOURTH session to find src/sound.c stale (old 10-pin body) —
 * always diff src against this file first (tmp/grind/func_80047A90/s5/
 * apply_candidate.py re-applies it).
 *
 * THIS FORM IS APPLIED IN src/sound.c (pure C, zero pins, zero rules).
 *
 * Session-3 change vs the s2 8-form: the loop-2 init is now K-FIRST source order
 * (k = 1; pt2; pt1;) — matching target's emission order at insns 30-33 — with
 * `k = 1;` wrapped in a single-level do { } while (0). Family:
 * .claude/rules/do-while-zero-exception.md (owner ruling 2026-07-06: sanctioned
 * pure-C match device for ANY codegen effect incl. register allocation; inline
 * FAKE annotation mandatory — present in src). Mechanism (measured, final6.lreg):
 * flow.c counts refs weighted by loop_depth, so the wrap's loop notes give the
 * k = 1 set weight 2 => k 5->6 weighted refs, prio 2*6/42 = .286 — above
 * pt1 (5/40 = .25) and pt2 (5/41 = .244), below a3off (6/33 = .364). Allocation
 * order stays pa1(a1) pa2(a2) a3off(a3) k(t0) pt1(t1) pt2(t2) while the emitted
 * init order becomes k=1 FIRST then the lui/addiu/addiu pt2/pt1 triple = target.
 * Without the wrap, k-first order = 18 (k drops to .233, pt1 steals t0 — measured
 * s2 and re-derived s3); with it both 30-33 slots close: 8 -> 6.
 *
 * Remaining 6 = the loop-1 2-cycle ONLY: i (pseudo 81: 7 refs/25 live -> a2,
 * wants t0) <-> judge LICM base (pseudo 94: 3/48 -> a3, wants a2), slots
 * 1,2,3,12,14,27. This residue is closed to the ENTIRE structural/priority axis:
 * s2 killed every separate-counter and merged-counter spelling; s3 additionally
 * killed declaration order (fully reversed = no change) and derived that a
 * wrap-weighted a2-blocker needs 6-9 nesting levels (unjustifiable under the
 * nested-wrap documentation duty). Next frontier is forensics: find_reg's
 * hard-reg walk for pseudo 81 (why does target's compile skip a2/a3?), the
 * local_alloc giv-status flip, and a permuter sweep from this 6-floor base.
 */
void func_80047A90(void) {
    s32 i;
    s32 k;
    s32 a3;
    s32 v1;
    s32 a0;
    s32 *pt2;
    s32 *pt1;
    s32 *pa1;
    s32 *pa2;
    s32 *pt3;
    s32 *temp;

    for (i = 0; i < 0x11; i++) {
        D_800EF59C[i] = ((s32)Judge[D_800EF558[i] & 0xFFF] * 0x271) >> 10;
        D_800EF558[i] += 0x12;
    }

    do {
        /* FAKE: loop-note ref weighting keeps k in $t0 while k=1 is
         * emitted before the pt2/pt1 lui pair, matching target order */
        k = 1;
    } while (0);
    pt2 = D_800EF59C;
    pt1 = D_800EF59C + 0x11;
  outer_loop:
    pa1 = pt1;
    a3 = 0;
    pa2 = pt2;
    pt3 = pt1 + 0x11;
  inner_loop:
    a0 = 0x7D0 - (*pa1 - *pa2);
    if (a0 < 0) {
        v1 = (a0 + 0xF) >> 4;
    } else {
        v1 = a0 / 10;
    }
    *pa1 += v1;
    if (k == 8) {
        *(s32 *)((s8 *)g_snd_fade_curve + a3) = v1;
        pa1++;
        a3 += 4;
        pa2++;
    } else {
        /* FAKE: loop tail duplicated into both arms (cross-jump re-merges,
         * byte-neutral); reg_n_refs lift lands pa2->$a2, a3->$a3, k->$t0 */
        pa1++;
        a3 += 4;
        pa2++;
    }
    if ((s32)pa1 < (s32)pt3)
        goto inner_loop;
    pt2 += 0x11;
    k++;
    pt1 += 0x11;
    if (k < 9)
        goto outer_loop;

    temp = (s32 *)D_800A3820;
    D_800A3820 = (s32)(temp + 1);
    *temp = (s32)&D_800EF070;
}
