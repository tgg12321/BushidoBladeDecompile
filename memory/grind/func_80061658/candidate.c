/* func_80061658 -- MATCHED, honest pure C (s5, 2026-08-25).
 * engine sandbox func_80061658 --disable all = 0 (target_insns 46, build_insns 46,
 * rules_dropped 0, zero pins / zero __asm__ in this function); full-tree
 * verify-oracle with this body in src/text1b.c = 62efab4f73f992798c43e8c730aa43baa10bb4fa
 * == oracle. canonical verdict = C.
 *
 * WHAT CLOSED IT -- the cluster idiom, not a coercion. This function is a
 * structural TWIN of func_80061710 (src/text1b.c:3358), which was completed as
 * COMPLETED-C in 28f49ad0. Cloning that sibling's exact shape closes 61658:
 *   (a) the switch arms set only `val` + `q` and the DEFAULT arm is
 *       `goto done;`, so the arms share ONE tail (`*q = 0; D_800F1180 = (s32)q;
 *       *v1 = val;`) -- this is the target's `j .L800616B4` cross-jump join, and
 *       it is why the `val` local is semantically REQUIRED (a shared tail cannot
 *       read a constant that was materialized inside an arm; inlining the
 *       constants measures 12 -- rejected/walkptr-inlined-switch-constants-12.c);
 *   (b) the post-call block is the WALKING-POINTER idiom
 *       `p = arg0; D_800F1140 = *p++; D_800F1144 = *p++; D_800F1148 = *p;`
 *       with the mask stored as a plain constant afterwards. Five COMPLETED-C
 *       functions in this same file already ship this exact idiom
 *       (func_800611A4:3205, func_8006133C:3232, func_800613C8:3247,
 *       func_800617C8:3403, func_80061710:3365).
 *
 * WHY IT BEATS THE s1-s4 WALL. s1-s3 proved the floor-9 residual was a pure
 * v0<->v1 swap caused by local-alloc.c:472: the shared `t = arg0[i]` load-temp
 * has reg_n_deaths==3 so it is DEFERRED to global_alloc, while the 1-death mask
 * constant is LOCAL and (MIPS has no REG_ALLOC_ORDER) takes the lowest free
 * caller-saved reg, $v0. The walking pointer removes the shared load-temp
 * entirely: each `*p++` result is its own single-death, single-block pseudo, so
 * the load temps are LOCAL and are allocated before the mask constant, take $v0,
 * and the mask -- live across the third load, hence conflicting -- is pushed to
 * $v1. That is the target allocation, reached by ordinary C rather than by any
 * copy-preference coercion. The s4 `val = 0x10FFFF; mask = val;` staging form
 * and the s5 single-variable-reuse form are BOTH unnecessary and are banked as
 * rejected (the owner refused that family for this function on 2026-07-27).
 *
 * CONSTRUCTS: exactly one FAKE-annotated construct -- the `s32 *v1 =
 * (s32 *)&D_800F116C;` pointer alias, the identical alias (with the identical
 * annotation) carried by the COMPLETED-C sibling at src/text1b.c:3366. Full
 * six-test vet in memory/grind/func_80061658/self_vet.md.
 */
void func_80061658(s32 *arg0, s32 arg1) {
    /* FAKE: local pointer alias to D_800F116C, mechanism: base-register
     * allocation / address-materialization caching in local-alloc (the alias
     * gives GCC one pseudo holding &D_800F116C, kept live in $a0 across the
     * switch instead of being re-materialized per use), lever-exhaustion:
     * memory/grind/func_80061658/hypotheses.md (s1-s3 structural + s4/s4b
     * permuter all measured dead on the direct-global form; identical alias
     * carried by the COMPLETED-C sibling func_80061710, src/text1b.c:3366). */
    s32 *v1 = (s32 *)&D_800F116C;
    s32 *p;
    u8 *q;
    s32 val;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    switch (arg1) {
    case 0:
        val = 0x21000C;
        q = &D_800F115C;
        break;
    case 1:
        val = 0x21000D;
        q = &D_800F115C + 1;
        break;
    default:
        goto done;
    }
    *q = 0;
    D_800F1180 = (s32)q;
    *v1 = val;
done:
    func_80060A68();
    p = arg0;
    D_800F1140 = *p++;
    D_800F1144 = *p++;
    D_800F1148 = *p;
    D_800A3464 = 0x10FFFF;
}
