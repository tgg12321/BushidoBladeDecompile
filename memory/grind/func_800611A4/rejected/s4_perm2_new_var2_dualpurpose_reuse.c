/* REJECTED — s4 permuter (interleaved-V0 seed) output-30-1. sandbox=5, 43/43.
 *
 * WHY DEAD: cheat-form. cheat-reviewer FAIL (2026-07-22). A freshly-INVENTED
 * local `new_var2` is dual-purposed across two unrelated statements (the
 * pre-call arg1+2 halfword store to sp[1], and the post-call mask store to
 * D_800A3464) purely to bias register allocation. This misapplies BOTH cited
 * carve-outs:
 *   - staged-value-reused-variable prereq #2 EXPLICITLY excludes inventing a
 *     new variable to have something to borrow; it requires reusing a variable
 *     the function ALREADY uses for a real job. new_var2 did not pre-exist.
 *   - defeat-licm-hoist-var-reuse is loop-scoped (loop.c move_movables); this
 *     function has NO loop, so the citation is a scope mismatch.
 *   - No /* FAKE */ annotation, no documented lever-exhaustion.
 *   - And it does not even close the match (sandbox stays 5).
 *
 * MECHANISTIC VALUE (kept as evidence, NOT as a candidate): routing both
 * values through new_var2 is the FIRST measured form that flips ALL THREE
 * post-call load-temps to $v0 (matching target exactly). The residual 5 diffs
 * are: mask 0xFFFFEF lands in $a0 (target wants $v1) = 3 insns, AND the
 * pre-call arg1+2 halfword regresses to $a0 (target $v0) = 2 insns. So the
 * load-temp->$v0 flip IS reachable, but every cheat-free lever that achieves
 * it also displaces the mask off $v1. Target needs load-temp=$v0 AND mask=$v1
 * simultaneously; no cheat-free form measured reaches both.
 */
void func_800611A4(s32 *arg0, s32 *arg1) {
    int new_var2;
    u16 sp[3];
    s32 t;
    u16 new_var;
    s32 *v1 = (s32 *) (&D_800F116C);
    sp[0] = *((u16 *) (((s32) arg1) + 0));
    new_var2 = *((u16 *) (((s32) arg1) + 2));
    sp[1] = new_var2;
    D_800A3468 = (s32) v1;
    new_var = *((u16 *) (((s32) arg1) + 4));
    D_800F117C = (s32) (&sp[0]);
    D_800F1178 = (s32) arg0;
    D_800F1180 = (s32) (&D_800F116A);
    *v1 = 0x21001A;
    sp[2] = new_var;
    func_80060A68();
    t = arg0[0];
    D_800F1140 = t;
    t = arg0[1];
    D_800F1144 = t;
    new_var2 = 0xFFFFEF;
    D_800A3464 = new_var2;
    t = arg0[2];
    D_800F1148 = t;
}
