/* REJECTED (s4b permuter, fresh-seed) — DENY-THE-COPY-SOURCE chassis.
 *
 * Hypothesis under test: is the s4 `val`-staging copy the UNIQUE score-0 lever,
 * or can a permuter reach a match through some OTHER (possibly sanctioned)
 * structure? To test, this chassis REMOVES the reused local `val` entirely by
 * inlining the switch constants (`*v1 = 0x21000C;` / `0x21000D;`), denying the
 * permuter the natural copy source it used in all 7 s4 finds.
 *
 * RESULT (campaign s4b-noval-freshseed, -j8, base_score 510, 30,762 iters,
 * ~16.7 min): best_new_score = 50 — the permuter re-discovered ONLY the pure
 * v0<->v1 register swap (9 differing insns) and NEVER reached 0. output-50-1
 * synthesized its own temp (`new_var`) but staged arg0/p through it (NOT the
 * mask constant), so mask stayed natural and the RA did not flip. No output-0-*
 * was produced.
 *
 * CONCLUSION: the score-0 basin is UNIQUELY the "stage the mask constant
 * 0x10FFFF through a copy source" coercion. There is no byte-neutral pure-C
 * form that reaches a match WITHOUT introducing that copy (matches the s3
 * GCC-source proof: find_free_reg has no lever to move mask off $v0 except a
 * copy-suggestion, and the constant has no natural copy source). Denying the
 * one convenient reused local drops the permuter to the pure-swap floor (50/9)
 * and it cannot recover. => the permuter modality is exhausted of any SANCTIONED
 * closing form; every score-0 form is the copy-coercion the Judge FAILed
 * 2026-07-22 22:50 (route-to-sign-off, pending owner SOTN-master census).
 */
void func_80061658(s32 *arg0, s32 arg1) {
    s32 *v1 = (s32 *)&D_800F116C;
    s32 t;
    s32 mask;
    u8 *p;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    switch (arg1) {
    case 0:
        p = &D_800F115C;
        *p = 0;
        D_800F1180 = (s32)p;
        *v1 = 0x21000C;   /* constant inlined; no reusable `val` local */
        break;
    case 1:
        p = &D_800F115C + 1;
        *p = 0;
        D_800F1180 = (s32)p;
        *v1 = 0x21000D;
        break;
    }
    func_80060A68();
    t = arg0[0]; D_800F1140 = t;
    t = arg0[1]; D_800F1144 = t;
    mask = 0x10FFFF;
    D_800A3464 = mask;
    t = arg0[2]; D_800F1148 = t;
}
