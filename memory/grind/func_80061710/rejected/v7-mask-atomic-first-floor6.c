/* func_80061710 — REJECTED (grind s2, structural modality, 2026-07-22)
 *
 * V7 "mask-atomic-first": pin-free, mask store hoisted before the 3 loads so
 * the mask pseudo dies before load 1. sandbox --disable all = 6 (46/46, 0 rules).
 *
 * WHY DEAD: 6 != 0, not a match. This is the BEST SANCTIONED (non-coercion,
 * 0-rule) floor for 710 — confirmed this session, matching sibling 611A4 s3.
 * The 6 residual is PURELY the schedule difference: this form computes mask as a
 * LEADING BLOCK (mask+all loads reuse $v0, conflict freed), but target INTERLEAVES
 * the mask lui/ori/sw between the load/store pairs. Any interleaved (target-schedule)
 * form makes mask live-across-loads -> local-alloc gives the longer-lived mask the
 * lower reg ($v0), loads get $v1 -> v0<->v1 wall = 9. No pure structure escapes this;
 * only the constant-staging copy-preference (candidate.c, policy-blocked) does.
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
    mask = 0x10FF10;
    D_800A3464 = mask;
    t = arg0[0]; D_800F1140 = t;
    t = arg0[1]; D_800F1144 = t;
    t = arg0[2]; D_800F1148 = t;
}
