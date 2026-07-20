/* REJECTED s2 — Variant A: drop `s32 *v1` pointer alias entirely.
 * Sandbox --disable all = 20 (much worse), build_insns=44 vs target 43.
 * Removing the alias forces two separate address computations for
 * `D_800F116C` (once for D_800A3468 store, once for the 0x21001A store) —
 * GCC can no longer share the lui/base. Adds an extra insn AND perturbs
 * RA far beyond the target's shape. Killed: the alias is load-bearing
 * for the address CSE, not a lever. */
void func_800611A4(s32 *arg0, s32 *arg1) {
    u16 sp[3];
    u16 new_var;
    s32 t;
    s32 mask;
    sp[0] = *((u16 *) (((s32) arg1) + 0));
    sp[1] = *((u16 *) (((s32) arg1) + 2));
    D_800A3468 = (s32) (&D_800F116C);
    new_var = *((u16 *) (((s32) arg1) + 4));
    D_800F117C = (s32) (&sp[0]);
    D_800F1178 = (s32) arg0;
    D_800F1180 = (s32) (&D_800F116A);
    D_800F116C = 0x21001A;
    sp[2] = new_var;
    func_80060A68();
    t = arg0[0];
    D_800F1140 = t;
    t = arg0[1];
    D_800F1144 = t;
    mask = 0xFFFFEF;
    D_800A3464 = mask;
    t = arg0[2];
    D_800F1148 = t;
}
