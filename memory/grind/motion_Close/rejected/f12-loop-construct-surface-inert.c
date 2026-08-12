/*
 * REJECTED — frontier F12 (session 10, synthesis): the walk's LOOP CONSTRUCT
 * surface is exhausted. Every real C loop construct other than the `goto` loop
 * already in candidate.c reproduces the do-while/while family's reference
 * accounting exactly, and none of them reaches score 13 at do-while(0) wrap
 * depth <= 2.
 *
 * WHY THIS WAS WORTH ASKING. Session 9 discovered that swapping the walk's loop
 * construct changes the WEIGHTING of every reference in the function, not just
 * the loop's own: flow.c weights REG_N_REFS by loop depth, and a `goto` loop
 * emits no NOTE_INSN_LOOP_BEG/END pair, so all references collapse to raw
 * counts. That moved the required wrap depth from 3 to 2. F12 asked whether any
 * OTHER honest construct moves the accounting further.
 *
 * MEASURED (tmp/grind/motion_Close/s10/f12sweep.py + .log; `sandbox
 * motion_Close --disable all` plus the instrumented cc1, BB2_ALLOC_DEBUG=1;
 * p assigned first and declared first in every cell; wrap depths 0/1/2):
 *
 *   construct                                      d0    d1    d2
 *   A  for (;;) { body; if (!count) break; }        20    20    20
 *   B  while (1) { body; if (!count) break; }       20    20    20
 *   C  for (;;) { body; if (count) continue; break;} 20   20    20
 *   D  for (; count != 0; ) { body; }               20    20    20
 *   G  goto tail-tested loop  (candidate.c)         20    20    13
 *   E  for (i = 0; i < 1; i++) { goto walk }        25    25    18
 *
 * A, B, C and D produce the IDENTICAL allocno table at every depth —
 * count 8 weighted refs / live_length 7 = 34285 and p 7/8 = 17500 (d0),
 * 8/8 = 30000 (d1), 9/8 = 33750 (d2) — i.e. exactly the do-while-LOOP family's
 * numbers. p is still 535 priority points short at depth 2 and needs depth 3,
 * which is the form sessions 5b-8 carried and which session 9's goto chassis
 * dominates. `continue` versus `break` versus a middle-clause test is
 * codegen-inert: all four emit one NOTE_INSN_LOOP_BEG/END pair around the same
 * body, so flow.c counts the same references at the same weight.
 *
 * E is measured only, never proposable. `for (i = 0; i < 1; i++)` is named in
 * the role prompt as a construct NOT sanctioned by the do-while(0) carve-out,
 * so it could not be submitted whatever it scored — and it scores WORSE
 * regardless: the induction variable becomes a third allocno (7 refs /
 * live_length 8 -> $s2), the function grows from 25 to 30 emitted instructions,
 * and the best cell is 18. An outer real loop is strictly dominated by a
 * do-while(0) wrap on both policy and measurement.
 *
 * CONSEQUENCE. With F10 (whole-TU shape, s8), F11 (global declarations, s9) and
 * now F12 (loop construct, s10) all measured inert, the only input to cc1 that
 * has ever moved this function is the body's statement/declaration order plus
 * the loop-note weighting device — and that axis bottoms out at 13, which the
 * s7 residual table accounts for entirely with three backend-level disproofs
 * (H1, F5, F7a/F7b). There is no unexplored C-side surface left.
 */

/* Representative dead form (A, depth 2 — score 20, allocno table identical to
   the do-while-loop family, so depth 3 would be required): */
void motion_Close(void) {
    void (**p)(void);
    s32 count;

    if (D_800A2668 != 0) {
        do { do { p = &D_8008D070; } while (0); } while (0);
        count = (s32)&D_00000000;
        if (count != 0) {
            for (;;) {
                void (*f)(void) = *p;
                p++;
                f();
                count--;
                if (count == 0) {
                    break;
                }
            }
        }
    }
}
