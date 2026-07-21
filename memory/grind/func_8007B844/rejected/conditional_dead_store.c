/* func_8007B844 — REJECTED FORM (conditional dead-store).
 *
 * This form was found by the directed permuter (~36k iters) and produces
 * sandbox --disable all == 0 AND retire SHA1 == oracle (drops all 6
 * regfix rules). It is FORBIDDEN per 2026-06-01 user policy + commit
 * 44ef3df (find_dead_conditional_stores detector wired) +
 * .claude/rules/no-new-park-categories.md "cheats by any spelling."
 *
 * Why it's a cheat (the 6-test checklist):
 *
 *   Test 1 — Semantic purpose: NONE. The conditional `new_var = ot;`
 *     inside `if (g_gpu_debug_level >= 2) { ... }` has zero observable
 *     effect — `new_var` is unconditionally overwritten by the
 *     subsequent `new_var = ot;` before any use of `new_var`. GCC's DCE
 *     removes the inner store from the emitted output.
 *
 *   Test 2 — Human-programmer test: NONE. No programmer writing this
 *     function from spec would write a conditional store that's
 *     immediately overwritten unconditionally. The construct exists
 *     SOLELY to influence GCC's RTL pseudo numbering / liveness analysis
 *     so the return-staging pseudo gets the chain depth cc1's sched.c
 *     requires for priority-4 scheduling.
 *
 *   Test 3 — GCC-internals justification: YES, exclusively. The
 *     mechanism description references sched.c INSN_PRIORITY, copy-prop
 *     preservation, and pseudo liveness — pure compiler-internals
 *     reasoning with no program-logic equivalent.
 *
 *   Test 4 — Permuter-found: YES. ~36k directed permuter iterations
 *     across 2 runs. Permuter cannot judge cheat-vs-not; this is exactly
 *     the case the "vet before surfacing" discipline covers.
 *
 *   Test 5 — Family: SAME as Lever D (dead-param-assign, forbidden
 *     2026-05-31). Same intent (DCE'd write to influence pre-DCE
 *     analysis), spelled with a local variable + conditional wrap.
 *     find_dead_conditional_stores detector catches the variant.
 *
 *   Test 6 — Naming announces intent: PARTIAL. `new_var` is permuter-
 *     auto-named; a worker writing this manually would likely use
 *     `_pre` or `_stage` — both coercion-pattern names.
 *
 * Detector status (2026-06-01, commit 44ef3df):
 *   engine/volatile_cheats.find_dead_conditional_stores catches this
 *   pattern. queue done refuses completion for any function carrying it.
 *
 * DO NOT re-derive this form (under any name: new_var, pre_ot, stage,
 * tmp, _dummy, etc.). DO NOT re-run the directed permuter from a base
 * that would re-discover it; if running the permuter, FIRST filter
 * output against the cheat catalog before measuring sandbox.
 */

u32 *func_8007B844(u32 *ot, s32 n) {
    u32 *new_var;
    if (g_gpu_debug_level >= 2) {
        g_gpu_debug_func(&D_80015F98, ot, n);
        new_var = ot;       /* CHEAT: conditional dead-store */
    }
    { u32 *v0 = g_gpu_dev_table; ((void(*)(u32*,s32))v0[11])(ot, n); }
    new_var = ot;           /* unconditionally overwrites */
    *new_var = ((u32)&g_gpu_ot_end) & 0xFFFFFF;
    return new_var;
}
