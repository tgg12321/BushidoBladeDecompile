/* Three byte tables (asm/data/7D920.data.s dlabels D_8009A830 / D_8009A838 / D_8009A840:
 * 8, 8 and 16 bytes). D_8009A838 is read signed (lb) here and in func_80058580; the
 * other two unsigned (lbu). */
extern u8 D_8009A830[];
extern s8 D_8009A838[];
extern u8 D_8009A840[];
/* ang_hosei_80056FE8 / func_80056FE8 -- angle-correction table lookup.
 *
 * MATCH-HACK FAMILY: duplicated-statement-into-arms
 * (.claude/rules/duplicated-statement-into-arms.md; frozen SOTN entry
 *  pre-slim-2026-10-01:.claude/rules/no-new-park-categories.md:285-293).
 * The single real statement "add this arm's angle adjustment into `base`" is
 * written once PER DISPATCH ARM instead of being cached in a temp and added
 * once after the join. Each copy is REAL on its path (prereq 1) and the copies
 * are re-merged byte-neutrally by post-reload cross-jumping (prereq 2).
 *
 * Lever-exhaustion (prereq 3): ffd7fef75^:memory/grind/func_80056FE8/hypotheses.md +
 * evidence.md, sessions s1-s7b -- structural reassociation (21 forms, s2/s3),
 * permuter (4 chassis, ~150k iters, s4/s5), copy-preference (s6: $a1 has no
 * ABI anchor in this 1-argument leaf, set_preference/global.c:1591 cannot
 * create one), scheduling wrappers (s6: do-while(0) at 3 placements never
 * shrinks base's live length), live-range shortening (s6: backfires -- raises
 * priority), register pins (s6: reschedule to 41 insns, 2 load-delay nops
 * lost). All measured dead.
 */
s32 func_80056FE8(PracticeMenuRec *arg0) {
    PracticeMenuRec *a2 = arg0->unk_00;
    s32 a3 = a2->unk_58[3];
    s32 base = a3 * 40;
    /* FAKE: `base += <arm value>` duplicated into all three dispatch arms
     * instead of a post-join combine; mechanism: GCC 2.7.2 jump2 post-reload
     * cross-jumping tail-merges the three copies into the target's single join
     * `addu $a1,$a1,$v0` (byte-neutral, build 43 == target 43), while the
     * reference-count lift flow.c records (reg_n_refs 4 -> 8) raises `base`'s
     * global.c allocno priority above the struct pointer's so global_alloc
     * colours `base` first (.greg `;; 3 regs to allocate: 77 73 72` ->
     * `77 in 5  73 in 6`) -- which the post-join spelling provably cannot
     * (`;; 4 regs to allocate: 82 73 77 72` -> `73 in 5  77 in 6`);
     * lever-exhaustion: see the ladder in this function's preamble comment and
     * ffd7fef75^:memory/grind/func_80056FE8/hypotheses.md s1-s7b. */
    if (a2->unk_A3 != 0xFF) {
        if (arg0->unk_5E == 0) {
            /* FAKE: duplicated copy (see above) */
            base += D_8009A830[a2->unk_0E] * 2;
        } else {
            /* FAKE: duplicated copy (see above) */
            base += D_8009A838[a2->unk_0E] * 8;
        }
    } else {
        /* FAKE: duplicated copy (see above) */
        base += D_8009A840[a2->unk_14] * 2;
    }
    return base + arg0->unk_00->unk_40A + 0x12C;
}
