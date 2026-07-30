/* func_800477E8 — MATCHED form as of grind session s4 (2026-07-30, permuter).
 *
 *   honest floor (sandbox --disable all): 0      (s1/s2 floor 17, s3 floor 5)
 *   build_insns == target_insns == 170, rules_dropped 2, strip_cheat_asm on.
 *
 * s3 left exactly one defect: tie A, the $t1/$t2 seating of `t1val`
 * (= gpu_CalcClut(0x10,0x1E0), the 4th call) against `t2` (= the 0x2C00
 * constant).  s4 closed it with a single `do { ... } while (0);` wrap around
 * loop1's body — the construct sanctioned by .claude/rules/do-while-zero-
 * exception.md (owner ruling 2026-07-06) and annotated inline below.
 *
 * WHY THE WRAP WORKS — measured, not inferred.  s3 established the closed-form
 * allocation model: GCC 2.7.2 global.c ranks allocnos by
 *      priority = floor_log2(n_refs) * n_refs * size / live_length
 * over the numbers the cc1 `-da` `.lreg` dump prints as
 * "Register NN used R times across L insns", and flow.c weights every
 * reference by the loop_depth that NOTE_INSN_LOOP_BEG establishes.  The wrap
 * emits a loop note, so every reference inside loop1's body gains one unit of
 * weight.  Measured on the .greg/.lreg dump of the real src/sound.c:
 *
 *              n_refs   live_length   priority        seat
 *   s3   t1val    3          76         0.0395   ->  $t2(10)   WRONG
 *        t2       5         150         0.0667   ->  $t1(9)    WRONG
 *   s4   t1val    4          76         0.1053   ->  $t1(9)    TARGET
 *        t2       7         148         0.0946   ->  $t2(10)   TARGET
 *
 * t1val has ONE in-loop reference and t2 has TWO (it is stored in both arms),
 * so the extra weight unit is worth +1 to t1val and +2 to t2 — and because
 * floor_log2 stays at 2 for both while t2's live_length is double t1val's,
 * the increment flips the comparison.  This is the same loop-note lever s3
 * used to unlock loop2; s1-s3 all searched for a *reference-count* answer to
 * tie A and the answer was a *weighting* answer.
 *
 * The wrap was found by decomp-permuter (session s4, chassis
 * tmp/grind/func_800477E8/s4/wsA, 911 iterations, permuter score 25 -> 0) and
 * then re-derived by hand against the allocation model above, so its mechanism
 * is understood rather than merely observed.
 *
 * NATURAL GEOMETRY WAS TRIED FIRST (do-while-zero prerequisite 2).  Three
 * sessions of honest restructures failed to move tie A, every one measured:
 * factoring the shared `*s0 = t2;` out of the two arms (correct seating but
 * 171 insns), caching the `a3 >= 5` condition in a local (171), inlining the
 * 0x2C00 literal in both arms (CSE rebuilds one pseudo, seating unchanged),
 * the literal in one arm only (171), routing both arms' store through a v0
 * copy (score 39), `s0[1] = t2;` hoisted with `s0 += 2` in the arms (167),
 * inverting the arms' branch sense (18), swapping which CLUT result each arm
 * stores (7), and converting loop1's inner goto-loop into a real do-while or
 * for (cc1 manufactures two extra induction pseudos that take $t1/$t2 and push
 * t1val/t2 out to $t3/$t4 — 10 and 13).  See memory/grind/func_800477E8/
 * rejected/ and hypotheses.md.
 *
 * The wrap is SINGLE-LEVEL: s4 also measured that HEAD's inherited
 * `do { v0 = v1 | a1; } while (0);` inside the `a3 & 1` arm is NOT needed —
 * removing it keeps the score at 0 — so it is gone and no nested-wrap
 * justification (prerequisite 3) is required.  The s1/s2/s3 "outstanding
 * un-annotated do-while(0)" Judge surface is therefore CLOSED.
 *
 * The rest of the form is s3's and is load-bearing; do not "simplify" it:
 *   - loop2 must be REAL loops (a do-while over the rows, a for over the
 *     columns).  m2c's goto shape emits no loop notes, starves every loop2
 *     pseudo of reference weight, and pins the wrong order.
 *   - loop2's row pointer and inner walking pointer must be COMPILER-MADE:
 *     index the row (`p = &D_800EF59C[a3 * 0x11]; p[a0] = val;`) so loop.c
 *     strength-reduces them.  loop.c puts iv initialisations after the source
 *     statements, which is why target's preheader initialises the row pointer
 *     LAST and why a source-level row pointer always swaps $a1/$a2.
 *   - loop2's row counter must be loop1's `a3`, the SAME C variable (a fresh
 *     local rotates the whole loop2 seating: 14 instead of 5), and it must be
 *     READ in the body — the `a3 * 0x11` index supplies that, without which
 *     check_dbra_loop reverses the loop and drops target's `slti $v0,$a3,0x9`.
 *   - loop1's inner loop must stay GOTO-shaped (see above).
 *   - `a0` stays shared across all three loops; splitting it costs 32-40.
 *   - the three pre-loop initialisations are in target's own order,
 *     `a3 = 0; t2 = 0x2C00; a0 = 0;`, worth one instruction.
 *   - `val = w;` must be an explicit local copied from the accumulator;
 *     letting LICM manufacture it from `a3 * 0x7D0` swaps $a1/$a2 (14).
 *   - declaration order is inert everywhere (the allocno-number tiebreak is
 *     only reached on EXACTLY equal priority, which no pair here has).
 */

s32 func_800477E8(void) {
    s16 *s0;
    s32 s3val;
    s32 s2val;
    s32 s1val;
    s32 t1val;
    s32 a3;
    s32 a0;
    s32 a2;
    s32 a1;
    s32 t0;
    s32 v1;
    s32 t2;
    s32 v0;
    s32 *ptr;
    s32 *p;
    s32 w;
    s32 val;

    s0 = D_800A33D0;
    s3val = gpu_CalcTPage(0, 0, 0x2C0, 0x1C0);
    s2val = gpu_CalcTPage(0, 0, 0x2C0, 0x180);
    s1val = gpu_CalcClut(0x10, 0x1E0);
    t1val = gpu_CalcClut(0x10, 0x1E0);
    a3 = 0;
    t2 = 0x2C00;
    a0 = 0;
    do {
        t0 = 0x1200;
        a2 = 0x13;
        /* FAKE: single-level do-while(0) wrap. Its loop note adds one unit of
         * loop_depth reference weight to everything in this body, which seats
         * t1val in $t1 and the 0x2C00 constant in $t2 as target has them
         * (without it: t1val 3 refs/76 insns loses to t2's 5/150). */
        do {
        a1 = 0;
        v1 = 1;
inner:
        if (a3 >= 5) {
            *s0 = s3val;
            s0 += 1;
            *s0 = t2;
            s0 += 1;
            *s0 = s1val;
            s0 += 1;
            v0 = -0xC1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x100;
            *s0 = v0;
            s0 += 1;
            v0 = -0x3FC1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x4000;
        } else {
            *s0 = s2val;
            s0 += 1;
            *s0 = t2;
            s0 += 1;
            *s0 = t1val;
            s0 += 1;
            v0 = -0x40C1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x4100;
            *s0 = v0;
            s0 += 1;
            v0 = -0x7FC1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x8000;
        }
        *s0 = v0;
        s0 += 1;
        if (a3 & 1) {
            v0 = a2 | t0;
            *s0 = v0;
            s0 += 1;
            v0 = v1 | a1;
        } else {
            v0 = v1 | a1;
            *s0 = v0;
            s0 += 1;
            v0 = a2 | t0;
        }
        *s0 = v0;
        s0 += 1;
        *s0 = 0;
        s0 += 1;
        *s0 = 0;
        s0 += 1;
        v0 = -0x1000;
        *s0 = v0;
        s0 += 1;
        t0 += 0x100;
        a2 += 1;
        a1 += 0x100;
        a0 += 1;
        v1 += 1;
        if (a0 < 0x10) goto inner;
        a3 += 1;
        } while (0);
        a0 = 0;
    } while (a3 < 8);

    {
        s32 *a0p;
        a0p = (s32 *)&D_800EF070;
        *(s8 *)a0p = 0xE;
        D_800EF07A = 4;
        D_800EF0BC = -0x2EE0;
        D_800EF071 = 0;
        D_800EF0C0 = 0;
        D_800EF0C4 = -0xFA0;
        D_800EF080 = 0;
        D_800EF082 = 0;
        D_800EF084 = 0;
        D_800EF078 = 0;
        D_800EF07C = 0;
        D_800EF076 = 0;
        func_800417D0(a0p);
    }

    a3 = 0;
    w = 0;
    do {
        val = w;
        p = &D_800EF59C[a3 * 0x11];
        for (a0 = 0x10; a0 >= 0; a0--) {
            p[a0] = val;
        }
        w += 0x7D0;
        a3 += 1;
    } while (a3 < 9);

    a0 = 0;
    ptr = &D_800EF558[0];
loop3:
    *ptr = (a0 << 7) & 0xFFF;
    a0++;
    ptr++;
    if (0x11 > a0) goto loop3;

    return (s32)s0 - (s32)D_800A33D0;
}
