/* func_80061710 — REJECTED (grind s2, structural modality, 2026-07-22)
 *
 * Middle-load block-local split: pin-free, the 2nd load wrapped in a { } scope.
 * sandbox --disable all = 7 (46/46, 0 rules).
 *
 * WHY DEAD: 7 != 0, not a match; and worse than the V7 floor of 6. Confirmed
 * this session matching sibling 658 s2. The block scope shortens t2's live range
 * but does not flip the interleaved-mask v0<->v1 allocation. See
 * v7-mask-atomic-first-floor6.c for the mechanism and structural_measurements.txt.
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
    { s32 t2 = arg0[1]; D_800F1144 = t2; }
    mask = 0x10FF10;
    D_800A3464 = mask;
    t = arg0[2]; D_800F1148 = t;
}
