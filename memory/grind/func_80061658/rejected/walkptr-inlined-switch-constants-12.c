/* REJECTED (s5, 2026-08-25) — sandbox --disable all = 12 (46/46 insns).
 *
 * Form: the sibling walking-pointer tail (`p = arg0; D_800F1140 = *p++; ...;
 * D_800A3464 = 0x10FFFF;`) but with the switch-arm constants written inline
 * (`*v1 = 0x21000C;` in each arm) instead of through the `val` local.
 *
 * WHY IT FAILS: without `val` the arms materialize the 0x21000C/D constants
 * directly into the store, so no single pseudo carries the value across the
 * arm join; the shared tail (`sb $zero,0($q); sw $q,D_800F1180; sw val,0($a0)`)
 * cannot be cross-jumped into one block and the arm code diverges. `val` is
 * therefore LOAD-BEARING and semantically required by the target's shared-tail
 * control flow — see candidate.c. The same body WITH `val` measures 0
 * (s5 H3/H4).
 */
void func_80061658(s32 *arg0, s32 arg1) {
    s32 *v1 = (s32 *)&D_800F116C;
    s32 *q;
    u8 *p;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    switch (arg1) {
    case 0:
        p = &D_800F115C;
        *p = 0;
        D_800F1180 = (s32)p;
        *v1 = 0x21000C;
        break;
    case 1:
        p = &D_800F115C + 1;
        *p = 0;
        D_800F1180 = (s32)p;
        *v1 = 0x21000D;
        break;
    }
    func_80060A68();
    q = arg0;
    D_800F1140 = *q++;
    D_800F1144 = *q++;
    D_800F1148 = *q;
    D_800A3464 = 0x10FFFF;
}
