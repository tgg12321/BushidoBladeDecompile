/* func_80061658 — BYTES-PROVEN pure-C form (honest sandbox --disable all = 0,
 * rules_dropped 0, zero pins/__asm__ this func). Found by the s4 directed
 * permuter (7 independent score-0 finds). PENDING OWNER RULING — see
 * tmp/grind/outcome_func_80061658.json (result: ruling-request) and the s4
 * evidence entry. NOT yet accepted; src is reverted to the HEAD pinned form
 * until the owner classifies the construct.
 *
 * THE LEVER (all 7 finds agree): stage the mask constant 0x10FFFF through the
 * ALREADY-USED local `val` (`val = 0x10FFFF; mask = val;`) instead of the
 * natural `mask = 0x10FFFF;`. `val` already holds 0x21000C/D in the switch and
 * is dead after it; reusing it to carry the mask gives `mask = val` a COPY
 * source. Net RA effect (permuter-measured, matches s1-s3 RTL): mask lands in
 * $v1 and the reused 3-death load-temp `t` wins $v0 — the exact target
 * allocation that the pins forced. Placement of `val = 0x10FFFF;` among the
 * loads varies across finds; the copy chain is the invariant.
 *
 * MECHANISM (why it flips the s1-s3 wall): the wall was local-alloc.c:472 —
 * mask (reg_n_deaths==1) → local_alloc → grabs $v0 (lowest free reg, MIPS has
 * no REG_ALLOC_ORDER, no copy-suggestion); the 3-death load-temp → global_alloc
 * → leftover $v1. Staging through `val` makes `mask` a copy of a reused pseudo,
 * so mask carries a copy-preference and val's multi-death changes the class
 * disposition — steering find_free_reg off $v0 for mask. This is precisely the
 * "copy-suggestion" escape the s2/s3 frontier named as the unmet need.
 *
 * CLASSIFICATION UNCERTAINTY (why ruling-request, not candidate-ready):
 * live code (no dead store), FAKE-annotated, lever-exhaustion documented (s1-s3)
 * — it is in the SPIRIT of the SOTN-sanctioned "variable reuse for RA control"
 * family (`randy = basePoint.x; baseX = randy;`). BUT layer-1 cheat-reviewer
 * FAILed it: the existing BB2 sanctions (staged-value-reused-variable = sched.c
 * priority mechanism, staging a LOAD; defeat-licm-hoist-var-reuse = loop-scoped)
 * do NOT cover staging a CONSTANT through a reused local to change local-alloc
 * register CHOICE on STRAIGHT-LINE code. That is arguably a new technique-family
 * application requiring an SOTN evidence pass + owner sign-off per
 * review-discipline-before-commit. Cannot self-approve (prime directive).
 */
void func_80061658(s32 *arg0, s32 arg1) {
    s32 *v1 = (s32 *)&D_800F116C;
    s32 t;
    s32 mask;
    u8 *p;
    s32 val;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    switch (arg1) {
    case 0:
        val = 0x21000C;
        p = &D_800F115C;
        *p = 0;
        D_800F1180 = (s32)p;
        *v1 = val;
        break;
    case 1:
        val = 0x21000D;
        p = &D_800F115C + 1;
        *p = 0;
        D_800F1180 = (s32)p;
        *v1 = val;
        break;
    }
    func_80060A68();
    t = arg0[0]; D_800F1140 = t;
    t = arg0[1]; D_800F1144 = t;
    val = 0x10FFFF; /* FAKE: stage mask const through reused `val` so the copy
                       lands mask in $v1 and the load-temp wins $v0 (local-alloc
                       copy-preference); structural axis exhausted s1-s3. */
    mask = val;
    D_800A3464 = mask;
    t = arg0[2]; D_800F1148 = t;
}
