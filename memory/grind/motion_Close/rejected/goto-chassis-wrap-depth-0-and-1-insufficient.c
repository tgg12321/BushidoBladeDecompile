/*
 * REJECTED — on the session-9 goto-loop chassis, do-while(0) wrap depths 0 and
 * 1 do NOT reach the floor. Both score 20; depth 2 scores 13.
 *
 * This file exists to satisfy prerequisite 3 of
 * `.claude/rules/do-while-zero-exception.md` for the NESTED (two-level) wrap
 * carried by memory/grind/motion_Close/candidate.c: a nested wrap requires a
 * written measurement showing that fewer levels are insufficient. It also
 * records why depth <= 1 is not merely unfound but structurally out of reach on
 * this chassis.
 *
 * MEASURED LADDER (tmp/grind/motion_Close/s9/gotosweep.py, sandbox
 * `motion_Close --disable all` + instrumented cc1 BB2_ALLOC_DEBUG=1):
 *   depth 0   p 4 refs / live_length 8 = 10000 -> $s1 ; count 5/7 = 14285 -> $s0   score 20
 *   depth 1   p 5/8 = 12500             -> $s1 ; count 5/7 = 14285 -> $s0   score 20
 *   depth 2   p 6/8 = 15000             -> $s0 ; count 5/7 = 14285 -> $s1   score 13
 * Declaration order of the two locals is inert at every depth (both orders give
 * the same score and the same table), so unlike session 3's tie-punning form
 * this win does not depend on global.c's allocno-number tie-break.
 *
 * NOT A SPELLING ARTEFACT. The same ladder was re-measured across five distinct
 * pointer/loop idioms — `*p++`, `p[0]` + separate bump, `--count` in place of
 * `count--`, decrement-before-call, and call-through `p[-1]()` with no `f`
 * temp (tmp/grind/motion_Close/s9/idiomsweep.py). Every idiom scores 20 at
 * depths 0 and 1 and 13 at depth 2 (the call-through idiom is worse throughout:
 * 21/21/16). The requirement is a property of the function's reference
 * structure, not of any one body's spelling.
 *
 * WHY DEPTH <= 1 CANNOT BE REACHED HERE. The target's address-materialisation
 * order requires p to be assigned FIRST. That places count's birth one
 * statement later, so count carries 5 raw references over live_length 7
 * (= 14285) while p carries 5 over 8 (= 12500) — p is behind on live_length,
 * not on references, and adding a reference to p costs one live-range step too
 * unless it is inside p's existing range. Hence p must reach 6 WEIGHTED
 * references, i.e. two loop-note levels. The only measured way to lower count
 * to 4 references is to delete the outer `if (count != 0)` guard by entering
 * the walk through a top-tested `goto test;` loop; that does let p win at depth
 * 1, but it scores 14, because the target HAS that outer guard (gotosweep.py
 * d1_pdecl_pfirst_top / d1_cdecl_pfirst_top). Trading one residual point for
 * one wrap level is not an improvement, so it is not the candidate.
 *
 * FRONTIER STATUS. F4b's success criterion is score 13 at wrap depth < 3; that
 * is met at depth 2. Depth <= 1 stays open only in the sense that no C form
 * reaching it is known, and the session-9 measurements above say the only
 * direction left is lengthening count's live range without adding a count
 * reference — i.e. an intervening real computation, which the target does not
 * perform. F4b is close to dead and is cosmetic regardless: H1 bounds the whole
 * family away from distance 0.
 */

/* depth 1 — measured 20. Depth 0 is the same body with the wrap removed. */
void motion_Close(void) {
    s32 count;
    void (**p)(void);

    if (D_800A2668 != 0) {
        do { p = &D_8008D070; } while (0);
        count = (s32)&D_00000000;
        if (count != 0) {
        again:
            {
                void (*f)(void) = *p;
                p++;
                f();
                count--;
            }
            if (count != 0) {
                goto again;
            }
        }
    }
}
