/* REJECTED (s5, 2026-08-25) — measures sandbox --disable all = 0 but is the
 * POLICY-REFUSED family and must NOT be submitted.
 *
 * Form: delete the separate `mask` local and reuse the switch-arm local `val`
 * to carry 0x10FFFF (`val = 0x10FFFF; D_800A3464 = val;`). No staging copy —
 * mechanistically distinct from the s4 `val = 0x10FFFF; mask = val;` form
 * (that one worked via a local-alloc COPY-preference; this one works because
 * making `val` multi-block sends it to global_alloc where it conflicts with the
 * local `p`@$v0 in the switch arms and is pushed to $v1).
 *
 * WHY REJECTED: it is still "borrow an existing live local for a second,
 * semantically unrelated value in order to steer register CHOICE" — the family
 * the owner REFUSED for this exact function on 2026-07-27 (docs/grind/decisions.md
 * 2026-07-27 func_80061658 entry, option (b)) and that the 2026-08-18 F1 survey
 * rated WEAK. Recorded here as a KILLED policy axis and as mechanism evidence,
 * not as a candidate. The accepted s5 form (candidate.c) needs no such reuse:
 * `val` there carries exactly one value to the shared tail.
 */
void func_80061658(s32 *arg0, s32 arg1) {
    s32 *v1 = (s32 *)&D_800F116C;
    s32 t;
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
    val = 0x10FFFF;
    D_800A3464 = val;
    t = arg0[2]; D_800F1148 = t;
}
