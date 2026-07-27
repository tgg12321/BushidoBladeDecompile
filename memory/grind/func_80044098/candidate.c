/* func_80044098 — s3 best form (pY): sandbox --disable all = 3 (26/26 insns).
 * Peel + hdr-split + holder BORROWED into the dead hdr local. The s2 sched1
 * stub is GONE: the fresh m2 holder was single-set, and sched.c
 * adjust_priority -> birthing_insn_p (reg_n_sets[dest]==1, dest live,
 * n_deaths always 0 because REG_DEAD notes are stripped) boosts such insns
 * to LAUNCH_PRIORITY when launched by the block-end jump, sinking the li to
 * the peel-block tail where it blocks cross-jump's backward suffix match.
 * hdr's 2nd set turns the boost off; the li schedules first (LUID order) and
 * cross-jump re-merges the ENTIRE peel: structure, schedule, li placement all
 * exactly target.
 * Residual 3 diffs = ONE register story: hdr's load segment. The merged
 * hdr pseudo (load+test+mask ∪ holder) lives through the loop, so it
 * conflicts with loop-temp($v0) and counter($a0) and lands $a1:
 *   lw $5,-4($3)   vs target lw a0    (dest)
 *   andi $2,$5     vs target andi v0,a0 (src)
 *   andi $4,$5     vs target andi a0,a0 (src)
 * Proven floor within both known holder families (s3 theorem): every
 * borrowable host's non-holder segment is target-pinned to $a0 or $v0, both
 * of which conflict with a loop-spanning holder segment (counter / loop
 * temp), and target writes $a1 exactly once (the li itself) — so a
 * multi-set holder can never byte-match all its segments; a fresh holder is
 * always boosted (stub 3, s2 pU form, kept in this file's history). Both
 * families bottom at 3.
 * Families: duplicated-statement-into-arms (peel) +
 * staged-value-reused-variable x named-local-fake-exception (borrowed
 * constant-holder — hybrid; flag for judge/owner classification if a 0
 * lands from this base). */
void func_80044098(s16 a0) {
    s32 *v1;
    s32 hdr;
    s32 a4;
    s32 *a6;

    v1 = D_80103608[a0];
    hdr = *(v1 - 1);
    a6 = v1 - 1;
    if (hdr & 0x8000) {
        a4 = hdr & 0x7FFF;
        *(v1 - 1) = a4;
        a4 = a4 - 1;
        if (a4 != -1) {
            /* FAKE: loop-const holder staged through the already-dead hdr
               (staged-value-reused-variable x named-local-fake-exception).
               mechanism: sched.c adjust_priority -> birthing_insn_p
               (reg_n_sets==1) LAUNCH_PRIORITY boost sinks a fresh holder's li
               to the peel-block tail, blocking cross-jump's suffix merge;
               hdr's 2nd set turns the boost off so the li schedules first and
               the peel fully re-merges (26/26). liveness: hdr's header value
               is dead after the 0x7FFF mask; -1 is live through the loop
               test. lever-exhaustion: memory/grind/func_80044098 s2-s3. */
            hdr = -1;
            /* FAKE: first-iteration peel duplicated from the loop body
               (duplicated-statement-into-arms); flow counts +4 pointer refs
               (16@23 pri 27826 > counter 14@18 23333) flipping
               pointer->$v1/counter->$a0; cross-jump re-merges all of it */
            *v1 -= (s32)a6;
            v1++;
            a4--;
            while (a4 != hdr) {
                *v1 -= (s32)a6;
                v1++;
                a4--;
            }
        }
    }
}
