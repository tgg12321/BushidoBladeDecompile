/* REJECTED (s5, 2026-08-25) - sandbox --disable all = 5 (build_insns 47 vs target 46).
   The accepted body without the `s32 *v1 = (s32 *)&D_800F116C;` pointer alias:
   &D_800F116C is re-materialized instead of being cached in one pseudo held in
   $a0 across the switch, costing one extra insn and 5 differing insns. This is
   the pointer-alias-fake-exception prerequisite-1 measurement (direct-global
   form measured negative FIRST) for func_80061658; the COMPLETED-C sibling
   func_80061710 measured the identical 5 for its own no-alias form. */
void func_80061658(s32 *arg0, s32 arg1) {
    s32 *p;
    u8 *q;
    s32 val;
    D_800A3468 = (s32)&D_800F116C;
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
    D_800F116C = val;
done:
    func_80060A68();
    p = arg0;
    D_800F1140 = *p++;
    D_800F1144 = *p++;
    D_800F1148 = *p;
    D_800A3464 = 0x10FFFF;
}
