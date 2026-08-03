/* func_80060544 — byte-proven form, re-submitted with the TWO COMMENT FIXES the
 * Judge's 2026-08-03 15:57 ruling demanded (grind session s8b, modality:
 * rederive).  NO CODE CHANGE from the form that ruling reviewed: the function
 * body below is byte-identical to
 * memory/grind/func_80060544/rejected/judge-fail-0803-1556.c's body except for
 * the two comments.
 *
 * HONEST CHEAT-FREE FLOOR: `sandbox func_80060544 --disable all` == **0**
 *   (18 at the start of s1, 4 at the end of s1, 2 from s2 through s7, 0 now;
 *    build_insns 133 == target_insns 133).  Re-verified THIS session with this
 *    exact file spliced into src/text1b.c.
 * Re-apply with `python3 tmp/grind/func_80060544/s3/apply_candidate.py`.
 *
 * ------------------------------------------------- WHY THIS IS A RE-SUBMISSION
 * The 15:57 ruling reads: "FAIL on one specific, cheap-to-fix defect, not on the
 * new construct.  THE CONSTRUCT ITSELF IS ACCEPTED IN SUBSTANCE. ...  a
 * re-submission with the two comment fixes and no code change will be
 * re-measured identically."  The two defects were:
 *   (1) the pre-loop `last = 3;` was re-submitted with NO /* FAKE */ annotation,
 *       against the standing instruction from the 13:11 ruling — fixed below at
 *       the assignment;
 *   (2) the `c3` FAKE comment called the line "the Case3 static-table handle"
 *       while it stores the address of D_8009B7D8 (the i == 0 table).  The
 *       stored value is arbitrary and nothing ever reads it; the comment now
 *       says exactly that — fixed below.
 * Nothing else changed.  Both fixes are comment-only, as instructed.
 *
 * --------------------------------------- WHAT s8b RE-MEASURED (independently)
 * This session was dispatched with a ledger digest that ended at s7, so it
 * re-derived the loop.c mechanism from scratch before finding the earlier s8
 * work in `rejected/`.  Two things came out of that which were NOT in the
 * ledger, both measured with the new sub-second gate
 * `tmp/grind/func_80060544/s8/gate.py` (whole-body variants -> emitted Case3
 * block, the la's dest pseudo + set count in .combine, the LAUNCH_PRIORITY
 * promotion, whether loop.c hoisted the la, instruction count):
 *
 *  A. LEVER BISECTION over the untouched m2c body at git HEAD (s8/mkbodies.py).
 *     L0 (m2c, three DIRECT `s.p_static = &D_800...;` stores)  a1-FIRST, la $3,
 *     in-loop, 118 insns; +geom split-init (L1) unchanged; +the shared `stat`
 *     carrier (L2) flips it to la-first with la $2.  Every combination WITHOUT
 *     L2 emits target's a1-first order; every combination WITH L2 does not.
 *     So s1's lever 2 is the SOLE cause of the residual the whole grind has been
 *     chasing — and it is simultaneously the only reason the la keeps $v0.
 *  B. WHY the direct-store form cannot simply be kept: with three compiler temps
 *     loop.c hoists all three `la`s into the preheader, they become long-lived
 *     pseudos, and global_alloc SPILLS them ("Spilling reg 3" in the .greg dump)
 *     so reload REMATERIALISES `la $3` at each use.  That is the true origin of
 *     the six `la $3 -> $2` regfix substs the sandbox drops: they were papering
 *     over a reload spill, not a plain allocation preference.  Target has ZERO
 *     `$v1` references in the whole function, so target's build never went
 *     through that spill — its arm addresses are ordinary in-block pseudos.
 *  C. The hoist-block lever is GENERAL, not carrier-specific: a mention of the
 *     Case3 address local in ANY later basic block blocks the hoist and fires
 *     the promotion at exactly 117 asm lines (== target's 133 insns) —
 *     `mid_off = c3;` after loop 1, the same in the tail, `geom = c3;`,
 *     `stat = c3;`, `end_off = c3;` all measured launch=1 / 117.  But every
 *     hoist-blocking mention that is itself LIVE costs instructions target does
 *     not have: c3 feeding loop 2's `p1` = 119, c3 read in the `Skip` join =
 *     120, and the earlier session's route B (the post-loop static-table read)
 *     = sandbox 11 / 132.  A live carrier must survive the loop in a callee-save;
 *     target's dies two instructions after the la.  That closes, from a third
 *     independent direction, the question of whether a dead-code-free spelling
 *     of this lever exists at target's instruction count.
 * Banked: rejected/live-hoist-block-uses-and-lever-bisection.c.
 * Fallback if the Judge rejects the family after all:
 * memory/grind/func_80060544/prior-floor2-candidate.c (the s2 form, distance 2).
 *
 * ------------------------------------------------------- THE MECHANISM, NAMED
 * `scan_loop` (tools/gcc-2.7.2/loop.c:693-701) treats an invariant SET as a
 * movable only if one of three cases holds: (1) `reg_in_basic_block_p`
 * (loop.c:1062) — the set is the register's FIRST mention and every use lies
 * between it and the end of that basic block; (2) the destination is a compiler
 * temp (not REG_USERVAR_P) and not the loop-test reg; (3) the set runs on every
 * iteration (`! maybe_never`).  For a named local assigned inside the Case3 arm,
 * (2) and (3) are already false, so (1) is the only route — and the dead
 * prologue store below makes it false by moving `regno_first_uid[c3]` off the
 * `la`.  The un-hoisted `la` then reaches sched1 inside the Case3 block as a
 * live, single-set pseudo (flow.c has deleted the dead store by then), which is
 * exactly what `adjust_priority` -> `birthing_insn_p` (sched.c:2496/2531/2601,
 * promotion to LAUNCH_PRIORITY at sched.c:3985) promotes — putting
 * `addu $a1,$zero,$zero` first in the block as target has it.  Without it,
 * `rank_for_schedule` falls through to `INSN_LUID (y) - INSN_LUID (x)` and the
 * a1 set-up, emitted last by expand_call, always wins.
 */
s32 func_80060544(s32 arg0, s32 arg1) {
    s32 geom;
    s32 c3;
    s32 stat;
    s32 last;
    S544 s;
    s32 end_off;
    s32 mid_off;
    s32 i;
    s32 j;
    s32 idx;
    s32 new_var6;
    s32 prev;
    s32 *p0;
    S544 *new_var2;
    s32 *p1;
    int new_var3;
    prev = arg0;
    mid_off = arg0 + 0x4EC;
    end_off = arg0 + 0x5F4;
    s.byte28 = 0;
    s.zero10 = 0;
    s.arg2_field = arg1;
    new_var3 = arg0 + 0x5DC;
    new_var6 = end_off;
    s.pad20 = 0x200;
    s.pad24 = 0x100;
    s.height = 0;
    s.width = 0;
    /* FAKE: dead store.  THE VALUE STORED HERE IS ARBITRARY AND IS NEVER READ —
     * it is not "the Case3 handle" and it is not the i == 0 table being set up
     * early; any value would do, and flow.c deletes the store outright, so this
     * line contributes NO instruction to the output (the build is 133 insns,
     * exactly target's count).  The line exists solely to give the pseudo an
     * earlier reference than its real assignment in the Case3 arm, before
     * loop.c runs: that moves `regno_first_uid[c3]` off the `la`, which makes
     * `reg_in_basic_block_p` return 0 at loop.c:700 and disqualifies
     * `c3 = (s32)(&D_8009B7D0);` as a movable (loop.c:693-701 — cases (2) and
     * (3) are already false for a named local assigned under `maybe_never`), so
     * the `la D_8009B7D0` is NOT hoisted into loop 1's preheader.  It therefore
     * reaches sched1 inside the Case3 block as a live pseudo with
     * reg_n_sets == 1 (this store having been deleted), and
     * adjust_priority()/birthing_insn_p() promote it to LAUNCH_PRIORITY
     * (sched.c:2496/2531/2601), which is what puts `addu $a1,$zero,$zero` first
     * in that block exactly as target has it.
     * Lever exhaustion: hypotheses.md s1-s8 — every C-level restructuring of the
     * block (s2/s3), ~97,000 permuter samples across three chassis (s4/s5), the
     * instrumented-compiler case analysis (s5/s6/s7), the s8 route table
     * (A/B/C), and the s8b re-measurement showing every LIVE hoist-blocking
     * mention costs +2/+3 instructions or lands the address in a callee-save.
     * Family: [[dead-store-fake-exception]]; mechanism family
     * [[defeat-licm-hoist-var-reuse]]. */
    c3 = (s32)(&D_8009B7D8);
    i = 0;
    /* FAKE: constant-holder for the special-cased last index.  It must sit
     * BETWEEN `i = 0;` and `idx = 0;` — that source position is what reproduces
     * target's prologue init order `$s0 = 0 / $s5 = 3 / $s1 = 0`
     * (asm/funcs/func_80060544.s prologue; hypotheses.md s1 H3: a loop.c-hoisted
     * CSE constant provably cannot land there, because move_movables emits
     * preheader movables immediately before the loop start, i.e. AFTER both
     * inits — which is exactly what the literal-3 spelling produced).  It is
     * read twice (`i == last`, `i != last`), so it is live, but it is still a
     * constant-holder and therefore carries this annotation per the 13:11 and
     * 15:57 rulings.  Lever exhaustion: hypotheses.md s1-s8.
     * Family: [[named-local-fake-exception]]. */
    last = 3;
    idx = 0;
    do {
        geom = (s32)(&D_8009B770);
        geom += idx;
        s.p_geom = (s32 *)geom;
        if (i < 3) {
            if (i > 0) {
                goto S800;
            }
            if (i == 0) {
                goto S7D8;
            }
            goto Skip;
        }
        if (i == last) {
            goto Case3;
        }
        goto Skip;
    S7D8:
        stat = (s32)(&D_8009B7D8);
        s.p_static = (s32 *)stat;
        goto Skip;
    S800:
        stat = (s32)(&D_8009B800);
        s.p_static = (s32 *)stat;
        goto Skip;
    Case3:
        c3 = (s32)(&D_8009B7D0);
        s.p_static = (s32 *)c3;
        s.pad0C = mid_off;
        mid_off = func_80073728(&s, 0);
    Skip:
        if (i != last) {
            s.arg1_field = prev;
            prev = func_8007352C(&s);
        }
        i += 1;
        idx += 0xC;
    } while (i < 4);
    s.p_geom = &D_8009B7A0;
    s.p_static = &D_8009B820;
    s.arg1_field = prev;
    new_var2 = &s;
    prev = func_8007352C(new_var2);
    j = 0;
    p1 = &D_8009B840;
    p0 = &D_8009B3B0;
    s.byte29 = 0xFF;
    s.byte2B = 0x10;
    s.byte2A = 0x10;
    s.byte28 = 1;
    do {
        s.p_geom = p0;
        s.p_static = p1;
        s.arg1_field = prev;
        prev = func_8007352C(&s);
        p1 = (s32 *)(((s32)p1) + 8);
        j += 1;
        p0 = (s32 *)(((s32)p0) + 0xC);
    } while (j < 2);
    initTexPage(new_var3, 1, 0, saMotionSet((s32)s.p_geom, 0), 0);
    ot_Link(D_800A374C + (arg1 * 4), new_var3);
    return new_var6 - arg0;
}
