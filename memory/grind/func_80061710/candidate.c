/* func_80061710 — BEST form: sandbox --disable all = 0 (46/46, zero rules/pins).
 *
 * *** THIS FORM IS POLICY-BLOCKED, NOT COMMITTABLE ***  (grind s1, 2026-07-22)
 *
 * The line `val = 0x10FF10; mask = val;` stages the mask constant through the
 * reused switch local `val`. It reaches BYTE-EXACT sandbox 0 — it is the ONLY
 * pure-C form (no register pins) that reproduces target's v0<->v1 layout
 * (load-web -> $v0, mask -> $v1). BUT the staging hop has no semantic purpose:
 * a human writing this from spec would never route the constant through `val`;
 * its only effect is the local-alloc.c:472 copy-preference that steers
 * find_free_reg off $v0 for the mask.
 *
 * This is the IDENTICAL construct the Judge FAILed for sibling func_80061658
 * (docs/grind/decisions.md 2026-07-22 22:50, route-to-sign-off) — a first-reach
 * "constant-staging through a reused live local to steer local-alloc register
 * CHOICE on straight-line code" coercion, outside every currently-sanctioned
 * family (staged-value-reused-variable is scheduler/order-scoped, not
 * register-choice; defeat-licm-hoist-var-reuse is loop-scoped). It is pending an
 * owner SOTN-master-branch census under func_80061658's OWNER-ESCALATION, which
 * explicitly names func_80061710 as a cluster member the ruling would resolve.
 *
 * DO NOT surface this as a match / candidate-ready. The best SANCTIONED
 * (non-coercion) floor is 9 (pin-free, pure v0<->v1 rename). See evidence.md.
 */
void func_80061710(s32 *arg0, s32 arg1) {
    s32 *v1 = (s32 *)&D_800F116C;
    s32 t;
    s32 mask;
    u8 *p;
    s32 val;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    switch (arg1) {
    case 0:
        val = 0x21000E;
        p = &D_800F115C + 2;
        *p = 0;
        D_800F1180 = (s32)p;
        *v1 = val;
        break;
    case 1:
        val = 0x21000F;
        p = &D_800F115C + 3;
        *p = 0;
        D_800F1180 = (s32)p;
        *v1 = val;
        break;
    }
    func_80060A68();
    t = arg0[0]; D_800F1140 = t;
    t = arg0[1]; D_800F1144 = t;
    val = 0x10FF10; mask = val;   /* constant-staging: reaches sandbox 0, POLICY-BLOCKED (see header) */
    D_800A3464 = mask;
    t = arg0[2]; D_800F1148 = t;
}
