/*
 * CANDIDATE — func_800401CC (src/text1a_pre.c), session s7 (forensics,
 * 2026-08-11). SANDBOX 2/78, verified this session, form APPLIED in
 * src/text1a_pre.c.
 *
 * STATUS: this is the BEST ADMISSIBLE form under the current bans. The
 * previous candidate (both masks staged through the two dead-after-call
 * texture-coordinate locals; sandbox 0/78, five independent verifications)
 * was BANNED by the latest layer-1 ruling as a respelling of the invented
 * constant-holder cheat; it is preserved verbatim in
 * rejected/banned-dual-staged-sandbox0.c. The same ruling directed reverting
 * to THIS score-2 v-staged-only chassis (its v-staging was ruled "properly
 * annotated and evidenced" by the first layer-1 review), and also banned
 * citing the staged-value-reused-variable family for any dual-mask staging.
 *
 * The 2 remaining diffs are the tail constant-load emission order only:
 *   ours:   li $7,0xff000000 ; li $6,0xff0000 ; ori $6,$6,0xffff
 *   target: li $6,0xff0000 ; ori $6,$6,0xffff ; li $7,0xff000000
 * All registers, all other instructions byte-identical.
 *
 * s7 FORENSIC RESULT (instrumented cc1, artifacts in
 * tmp/grind/func_800401CC/s2/forensics/): the residual is STRUCTURALLY
 * LOCKED for every admissible spelling. Named mechanism chain:
 *
 * 1. GCC 2.7.2 sched1 is a REVERSE list scheduler. When a scheduled insn
 *    frees a producer, adjust_priority (sched.c:2543) boosts the producer to
 *    LAUNCH_PRIORITY (0x7f000001) iff birthing_insn_p (sched.c:2504) — i.e.
 *    single-set REG dest (reg_n_sets==1). Boosted insns are picked earlier
 *    in reverse time = placed LATER (adjacent to consumers); unboosted
 *    constants sink to the block head.
 * 2. A 2-insn MIPS constant (lui+ori) is split by sched1's try_split into
 *    TWO sets of ONE pseudo -> reg_n_sets==2 -> NEVER boosted. A 1-insn
 *    block-local constant is ALWAYS boosted when freed. This asymmetry is
 *    invariant over all block-local spellings.
 * 3. Floor-7 (all-inline) chassis: high-mask li boosted, low-mask pair not
 *    -> pair born 4 luids earlier (birth 18 vs 22, deaths 48/46) ->
 *    qty_compare_1 (local-alloc.c:1660) priority 12/24 vs 12/30 -> high
 *    mask allocated first -> takes $6 (target: low mask in $6). Locked.
 * 4. THIS score-2 chassis: v is a multi-set global-alloc'd pseudo (head set
 *    + tail set) -> its li is unboosted (ADJPRI birth=0, measured); the
 *    low-mask pair is unboosted too -> all tie at priority 1 -> the
 *    rank_for_schedule LUID tiebreak (sched.c:2461) preserves RTL chain
 *    order -> v's set (which must dominate stmt1, since stmt1 reads v)
 *    always precedes stmt1's inline low-mask materialization -> li $7
 *    always emitted first. sched2 cannot fix it (adjust_priority is a no-op
 *    after reload). Locked for every source order (confirms K9 with
 *    mechanism).
 * 5. Corollary: the target byte order (low-mask pair BEFORE the high-mask
 *    li, with registers $6/$7 as in target) is reachable ONLY when BOTH
 *    masks live in multi-block (global-alloc'd) pseudos whose explicit sets
 *    control chain order — i.e. exactly the banned dual-staged shape. Every
 *    other axis is measured or analytically dead (H8, H12-H14, K1-K9).
 */
void func_800401CC(s32 a0, s32 a1, s32 a2) {
    s16 buf[4];
    u16 *tbl;
    s16 u;
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
        /* FAKE: OT-code mask staged through the dead v-coord local (its value
           was consumed by the draw call above; the mask is read by both
           packet-link statements below), mechanism: global.c allocno call-arg
           copy preference ($a3 from the pre-call (s16)v arg copy) keeps the
           multi-set pseudo at $a3, removing the high mask from the local-alloc
           qty pool, lever-exhaustion: memory/grind/func_800401CC/hypotheses.md
           K1-K4 + P1/P2 + s2-permuter A-probes + s7 forensic proof (sched.c
           adjust_priority birthing-boost asymmetry locks every block-local
           spelling) */
        v = 0xFF000000;
        *pkt = (*pkt & v) | (ot[0x3FFC / 4] & 0xFFFFFF);
        ot[0x3FFC / 4] = (ot[0x3FFC / 4] & v) | ((s32)pkt & 0xFFFFFF);
        D_800A3378 = (s32)(pkt + 6);
    }
}
