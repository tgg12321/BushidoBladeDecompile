/*
 * UNMEASURED LEAD (session 4) — READ THIS BEFORE ANYTHING ELSE IN SESSION 5.
 *
 * This is the best permuter find of the session-4 campaign on the policy-clean
 * chassis: tmp/grind/motion_Close/s4/wsA, `output-383-1`, permuter weighted
 * score **383** against that workspace's base of 468 — a far larger drop than
 * the 463 find that produced this session's floor-16 candidate. It was written
 * by the permuter 754 s after launch and only surfaced in the end-of-session
 * `harvest --stop` telemetry, by which point the WSL drvfs mount (and then the
 * whole WSL service) had failed, so **it was never re-measured with
 * `sandbox motion_Close --disable all`.**
 *
 * IT IS NOT A RESULT. The permuter's weighted metric (regs x5, reorderings x60,
 * ins/del x100) is not the engine distance, and this session measured the two
 * metrics disagreeing badly on this very function (the s3 chassis scores 835 to
 * the clean chassis' 468 while the engine ranks them 16 to 17). 383 could be a
 * genuine sub-16 form or it could be an artifact. FIRST ACTION FOR SESSION 5:
 * measure it, both as-is and with the two constructs separated.
 *
 * WHAT IT DOES. Two changes against the clean chassis:
 *   1. `count = D_800A2668 != 0;` — the guard is staged into `count` (which is
 *      then immediately overwritten with the table length inside the branch).
 *      This is a variable-reuse / staging spelling; it is the same idea as
 *      session 3's guard carry but through the s32 local instead of through a
 *      type-punned function-pointer-pointer, so it carries no cast.
 *   2. `D_800A2668++; D_800A2668--;` — a redundant increment/decrement pair on
 *      the guard global.
 *
 * POLICY WARNING — DO NOT SUBMIT THIS WITHOUT A RULING. Construct 2 is a first
 * reach of a family nothing in the frozen SOTN list covers. Test it honestly:
 *   - If GCC folds the pair away (likely: two adjacent RMWs on the same
 *     non-volatile location with nothing between them), it emits no bytes and
 *     its ONLY function is to move GCC's analysis — checklist T1 (no semantic
 *     purpose) and T2 (no human writes it) both fail, and it is a coercion by
 *     any spelling.
 *   - If it does NOT fold, it emits a store to a global that the target never
 *     stores to, which is a behavioural difference from the target, not a
 *     match.
 * Either way it is not submittable as-is. The right session-5 move is to
 * measure the two constructs SEPARATELY: construct 1 alone may carry most of
 * the gain and sits inside the sanctioned "variable reuse for codegen control"
 * family (.claude/rules/no-new-park-categories.md, SOTN-accepted techniques),
 * and it composes with this session's do-while(0) wrap. If construct 2 turns
 * out to be load-bearing, emit `ruling-request` — do not self-approve it.
 *
 * Reminder: H1 still caps the function (REG_PARM_STACK_SPACE=16 forces a
 * 32-byte frame against the target's 16), so even a large drop here cannot
 * reach distance 0.
 */

void motion_Close(void) {
    s32 count;
    void (**p)(void);

    count = D_800A2668 != 0;
    D_800A2668++;
    D_800A2668--;
    if (count) {
        count = (s32)&D_00000000;
        p = &D_8008D070;
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
