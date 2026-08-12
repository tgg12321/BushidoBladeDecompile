/*
 * REJECTED (session 5) — the p-first chassis with a do-while(0) wrap of depth
 * 1 or 2 around p's initialiser. Kept as the MEASURED JUSTIFICATION required by
 * `.claude/rules/do-while-zero-exception.md` prerequisite 3 (nested wraps need
 * a written demonstration that a single level was insufficient).
 *
 * MEASUREMENT (tmp/grind/motion_Close/s5/sweep6.log, instrumented cc1
 * BB2_ALLOC_DEBUG=1; count is pseudo 72, p is pseudo 73):
 *
 *   f1_pfirst_wrap1  score 20   p  8 refs / len 8 = 30000 -> $s1
 *                               count 8 / 7      = 34285 -> $s0
 *   f2_pfirst_wrap2  score 20   p  9 / 8         = 33750 -> $s1
 *                               count 8 / 7      = 34285 -> $s0
 *   f3_pfirst_wrap3  score 13   p 10 / 8         = 37500 -> $s0   <-- the candidate
 *                               count 8 / 7      = 34285 -> $s1
 *
 * Depth 1 and depth 2 leave count in $s0, so the loop body emits the wrong
 * registers and the score is 20 — worse than the count-first floor-16 form
 * that does not need a nested wrap at all. Only depth 3 crosses count's 34285.
 * Depths 4, 5 and 6 (f4/f5/g7) all also score 13 and buy nothing further, so
 * depth 3 is both necessary and minimal.
 *
 * The body below is the depth-1 variant, i.e. session 4's exact single-level
 * device merely moved above count's initialiser. It is what
 * `rejected/dw0-pfirst-order-inverts-priority.c` predicted and is retained
 * separately from that file because its VALUE is the priority table above, not
 * the negative result.
 */

void motion_Close(void) {
    s32 count;
    void (**p)(void);

    if (D_800A2668 != 0) {
        do { p = &D_8008D070; } while (0);
        count = (s32)&D_00000000;
        if (count != 0) {
            do {
                void (*f)(void) = *p;
                p++;
                f();
                count--;
            } while (count != 0);
        }
    }
}
